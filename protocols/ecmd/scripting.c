/* 
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "core/vfs/vfs.h"
#include "protocols/ecmd/parser.h"
#include "scripting.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_ECMD_SCRIPT
# include "core/debug.h"
# define SCRIPTDEBUG(a...)  debug_printf("ECMD script: " a)
#else
# define SCRIPTDEBUG(a...)
#endif

#define STR_EQUALS "eq"
#define STR_NOTEQUALS "ne"
#define OK "OK"
#define NOT "!"
#define EQUALS "=="
#define NOTEQUALS "!="
#define GREATER ">"
#define LOWER "<"
#define GREATEREQUALS ">="
#define LOWEREQUALS "<="

#if ECMD_INPUTBUF_LENGTH > 255
#error please adjust return type of readline() from uint8_t to uint16_t
#endif

typedef struct
{
  char value[ECMD_SCRIPT_VARIABLE_LENGTH];
} variables_t;

variables_t vars[ECMD_SCRIPT_MAX_VARIABLES];

typedef struct
{
  struct vfs_file_handle_t *handle;
  uint16_t linenumber;
  vfs_size_t filepointer;
} script_t;

script_t current_script;

static int16_t
to_many_vars_error_message(char *output, uint16_t len)
{
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("max var exceed %i"),
                ECMD_SCRIPT_MAX_VARIABLES));
}

// read a line from file "handle", stored in "line", starting at "pos"
static vfs_size_t
vfs_fgets(struct vfs_file_handle_t *handle, char *line, vfs_size_t pos)
{
  vfs_fseek(handle, pos, SEEK_SET);
  vfs_size_t readlen = vfs_read(handle, line, ECMD_INPUTBUF_LENGTH - 1);

  line[ECMD_INPUTBUF_LENGTH - 1] = 0;
  SCRIPTDEBUG("fgets (%i) : %s\n", readlen, line);
  vfs_size_t i = 0;
  while (i < readlen && line[i] != 0x0a)
  {
    i++;
  }
  line[i] = 0;
  return i;                     // size until linebreak
}

// read a line from script
static uint8_t
readline(char *buf)
{
  vfs_size_t len =
    vfs_fgets(current_script.handle, buf, current_script.filepointer);
  SCRIPTDEBUG("readline: %s\n", buf);
  if (len == -1 || len >= ECMD_INPUTBUF_LENGTH)
  {
    return 0;
  }
  current_script.filepointer += len + 1;
  current_script.linenumber++;
  return (uint8_t) len;
}

int16_t
parse_cmd_goto(char *cmd, char *output, uint16_t len)
{
  uint8_t gotoline = 0;
  char line[ECMD_INPUTBUF_LENGTH];

  if (current_script.handle == NULL)
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no script")));
  }
  sscanf_P(cmd, PSTR("%hhu"), &gotoline);

  SCRIPTDEBUG("current %u goto line %u\n", current_script.linenumber,
              gotoline);

  if (gotoline < current_script.linenumber)
  {
    SCRIPTDEBUG("seek to 0\n");
    vfs_fseek(current_script.handle, 0, SEEK_SET);
    current_script.linenumber = 0;
    current_script.filepointer = 0;
  }
  while ((current_script.linenumber != gotoline) &&
         (current_script.linenumber < ECMD_SCRIPT_MAXLINES))
  {
    SCRIPTDEBUG("seeking: current %i goto line %i\n",
                current_script.linenumber, gotoline);
    if (readline(line) == 0)
    {
      SCRIPTDEBUG("leaving\n");
      break;
    }
  }
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_exit(char *cmd, char *output, uint16_t len)
{
  if (current_script.handle == NULL)
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no script")));
  }
  vfs_close(current_script.handle);
  current_script.handle = NULL;
  current_script.linenumber = 0;
  current_script.filepointer = 0;
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_call(char *cmd, char *output, uint16_t len)
{
  char filename[10];
  char line[ECMD_INPUTBUF_LENGTH];
  uint8_t lsize = 0;
  uint8_t run = 0;
  vfs_size_t filesize;

  sscanf_P(cmd, PSTR("%s"), &filename); // should check for ".es" extention!
  current_script.handle = vfs_open(filename);

  if (current_script.handle == NULL)
  {
    SCRIPTDEBUG("%s not found\n", filename);
    return ECMD_FINAL(1);
  }

  filesize = vfs_size(current_script.handle);

  SCRIPTDEBUG("start %s from %i bytes\n", filename, filesize);
  current_script.linenumber = 0;
  current_script.filepointer = 0;

  // open file as long it is open, we have not reached max lines and 
  // not the end of the file as we know it
  while ((current_script.handle != NULL) &&
         (run++ < ECMD_SCRIPT_MAXLINES) &&
         (filesize > current_script.filepointer))
  {

    lsize = readline(line);
    SCRIPTDEBUG("(linenr:%i, pos:%i, bufsize:%i)\n",
                current_script.linenumber, current_script.filepointer, lsize);
    SCRIPTDEBUG("exec: %s\n", line);
    if (lsize != 0)
    {
      ecmd_parse_command(line, output, len);
    }
  }
  SCRIPTDEBUG("end\n");

  parse_cmd_exit(cmd, output, len);

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_cat(char *cmd, char *output, uint16_t len)
{
  char filename[10];
  char line[ECMD_INPUTBUF_LENGTH];
  uint8_t lsize = 0;
  uint8_t run = 0;
  vfs_size_t filesize;

  sscanf_P(cmd, PSTR("%s"), &filename); // should check for ".es" extention!
  current_script.handle = vfs_open(filename);

  if (current_script.handle == NULL)
  {
    SCRIPTDEBUG("%s not found\n", filename);
    return ECMD_FINAL(1);
  }

  filesize = vfs_size(current_script.handle);

  SCRIPTDEBUG("cat %s from %i bytes\n", filename, filesize);
  current_script.linenumber = 0;
  current_script.filepointer = 0;

  // open file as long it is open, we have not reached max lines and 
  // not the end of the file as we know it
  while ((current_script.handle != NULL) &&
         (run++ < ECMD_SCRIPT_MAXLINES) &&
         (filesize > current_script.filepointer))
  {

    lsize = readline(line);
    SCRIPTDEBUG("cat: %s\n", line);
  }

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_wait(char *cmd, char *output, uint16_t len)
{
  uint16_t delay;
  sscanf_P(cmd, PSTR("%u"), &delay);
  SCRIPTDEBUG("wait %ims\n", delay);
  while (delay--)
    _delay_ms(1);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_set(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  char value[ECMD_SCRIPT_VARIABLE_LENGTH];
  sscanf_P(cmd, PSTR("%hhu %s"), &pos, &value);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES)
  {
    return to_many_vars_error_message(output, len);
  }

  strcpy(vars[pos].value, value);
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("%%%i set to %s"), pos, vars[pos].value));
}

int16_t
parse_cmd_get(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%hhu"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES)
  {
    return to_many_vars_error_message(output, len);
  }
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), vars[pos].value));
}

int16_t
parse_cmd_inc(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%hhu"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES)
  {
    return to_many_vars_error_message(output, len);
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value + 1, vars[pos].value, 10);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_dec(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%hhu"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES)
  {
    return to_many_vars_error_message(output, len);
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value - 1, vars[pos].value, 10);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_if(char *cmd, char *output, uint16_t len)
{
  char cmpcmd[ECMD_SCRIPT_COMPARATOR_LENGTH];
  char comparator[3];
  char konst[ECMD_SCRIPT_COMPARATOR_LENGTH];
//  char cmd[]= "if ( whm != 00:01 ) then exit";
  uint8_t success = 0;          // default false

  sscanf_P(cmd, PSTR("( %s %s %s ) then "), &cmpcmd, &comparator, &konst);
  char *ecmd = strstr_P(cmd, PSTR("then"));
  if (ecmd == NULL)
  {
    SCRIPTDEBUG("cmd not found\n");
    return ECMD_FINAL(1);
  }
  ecmd += 5;

  SCRIPTDEBUG("ecmd: %s\n", ecmd);
  SCRIPTDEBUG("cmpcmd: %s\n", cmpcmd);
  SCRIPTDEBUG("comparator: %s\n", comparator);
  SCRIPTDEBUG("konst: %s\n", konst);

  // if cmpcmd starts with % it is a variable
  if (cmpcmd[0] == '%')
  {
    uint8_t varpos = cmpcmd[1] - '0';
    // get variable and set it to output
    strcpy(output, vars[varpos].value);
  }
  else
  {                             // if not, it is a command
    // execute cmp! and check output
    if (!ecmd_parse_command(cmpcmd, output, len))
    {
      SCRIPTDEBUG("compare wrong\n");
      return ECMD_FINAL(1);
    }
  }
  SCRIPTDEBUG("cmp '%s' %s '%s'\n", output, comparator, konst);

  // check comparator  
  if (strcmp(comparator, STR_EQUALS) == 0)
  {
    SCRIPTDEBUG("try " STR_EQUALS "\n");
    success = (strcmp(output, konst) == 0);
  }
  else if (strcmp(comparator, STR_NOTEQUALS) == 0)
  {
    SCRIPTDEBUG("try " STR_NOTEQUALS "\n");
    success = (strcmp(output, konst) != 0);
  }
  else
  {
    uint16_t outputvalue = atoi(output);
    uint16_t konstvalue = atoi(konst);
//    debug_printf("cmp atoi: %i %s %i\n", outputvalue, comparator, konstvalue);
    if (strcmp(comparator, OK) == 0)
    {
      SCRIPTDEBUG("try " OK "\n");
      success = outputvalue;
    }
    else if (strcmp(comparator, NOT) == 0)
    {
      SCRIPTDEBUG("try " NOT "\n");
      success = (outputvalue != 0);
    }
    else if (strcmp(comparator, EQUALS) == 0)
    {
      SCRIPTDEBUG("try " EQUALS "\n");
      success = (outputvalue == konstvalue);
    }
    else if (strcmp(comparator, NOTEQUALS) == 0)
    {
      SCRIPTDEBUG("try " NOTEQUALS "\n");
      success = (outputvalue != konstvalue);
    }
    else if (strcmp(comparator, GREATER) == 0)
    {
      SCRIPTDEBUG("try " GREATER "\n");
      success = (outputvalue > konstvalue);
    }
    else if (strcmp(comparator, LOWER) == 0)
    {
      SCRIPTDEBUG("try " LOWER "\n");
      success = (outputvalue < konstvalue);
    }
    else if (strcmp(comparator, GREATEREQUALS) == 0)
    {
      SCRIPTDEBUG("try " GREATEREQUALS "\n");
      success = (outputvalue >= konstvalue);
    }
    else if (strcmp(comparator, LOWEREQUALS) == 0)
    {
      SCRIPTDEBUG("try " LOWEREQUALS "\n");
      success = (outputvalue <= konstvalue);
    }
    else
    {
//      debug_printf("unknown comparator: %s\n", comparator);
      return ECMD_FINAL(3);
    }
  }
  // if compare ok, execute command after then
  if (success)
  {
    SCRIPTDEBUG("OK, do: %s\n", ecmd);
    if (ecmd_parse_command(ecmd, output, len))
    {
      SCRIPTDEBUG("done: %s\n", output);
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), output));
    }
    return ECMD_FINAL(2);
  }
  SCRIPTDEBUG("success was: %i\n", success);
  return ECMD_FINAL(1);
}

// do nothing but provides comments in scripting
int16_t
parse_cmd_rem(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL_OK;
}

// hello echo!
int16_t
parse_cmd_echo(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), cmd));
}

// if ECMD_SCRIPT_AUTOSTART_SUPPORT is enabled
// then call script by name ECMD_SCRIPT_AUTOSTART_NAME

// could not run on startup, or init, so make this run just once!
uint8_t ecmd_script_autorun_done = 0;

int16_t
ecmd_script_init_run(void)
{
#ifdef  ECMD_SCRIPT_AUTOSTART_SUPPORT
  if (ecmd_script_autorun_done == 1)
  {
    return ECMD_FINAL_OK;
  }
  ecmd_script_autorun_done = 1;
  char cmd[] = CONF_ECMD_SCRIPT_AUTOSTART_NAME;
  char output[ECMD_SCRIPT_VARIABLE_LENGTH];
  SCRIPTDEBUG("auto run: %s\n", cmd);
  return parse_cmd_call(cmd, output, sizeof(cmd));
#else
  return ECMD_FINAL_OK;
#endif
}

/*
  -- Ethersex META --
  block([[ECMDScript]])
  ecmd_feature(goto, "goto ",N, Goto line N in currently running script)
  ecmd_feature(exit, "exit",, Exit currently running script)
  ecmd_feature(wait, "wait ",I, Wait I milliseconds)
  ecmd_feature(set, "set ",VAR VALUE, Set variable VAR to VALUE)
  ecmd_feature(get, "get ",VAR, Get value of variable VAR)
  ecmd_feature(inc, "inc ",VAR, Increment variable VAR (a number) )
  ecmd_feature(dec, "dec ",VAR, Decrement variable VAR (a number) )
  ecmd_feature(call, "call ",FILENAME, Start script named FILENAME)
  ecmd_ifdef(DEBUG_ECMD_SCRIPT)
    ecmd_feature(cat, "cat ",FILENAME, cat file content (with debug only))
  ecmd_endif()
  ecmd_feature(if, "if ",( CMD/VAR == CONST ) then CMD2, If condition matches execute CMD2)
  ecmd_feature(rem, "rem",<any>, Remark for anything)
  ecmd_feature(echo, "echo ",<any>, Print out all arguments of echo)
  header(protocols/ecmd/scripting.h)
  ifdef(`conf_ECMD_SCRIPT_AUTOSTART',`timer(50,ecmd_script_init_run())')
*/
