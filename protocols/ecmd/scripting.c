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
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "core/vfs/vfs.h"
#include "protocols/ecmd/parser.h"
#include "scripting.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"

#include "protocols/ecmd/ecmd-base.h"


// read a line from file "handle", stored in "line", starting at "pos"
int16_t 
vfs_fgets(struct vfs_file_handle_t *handle, char *line, vfs_size_t pos){
    uint8_t i = 0;
    vfs_fseek(handle, pos, SEEK_SET);
    vfs_size_t readlen = vfs_read(handle, line, ECMD_INPUTBUF_LENGTH - 1);

    line[ECMD_INPUTBUF_LENGTH - 1] = 0;
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ECMD script: fgets (%i) : %s\n", readlen, line);
#endif // DEBUG_ECMD_SCRIPT
    for (i = 0; i < readlen ; i++){
       if (line[i] == 0x0a)  break;
    }
    line[i] = 0;
    return i; // size until linebreak
}

// read a line from script
uint8_t
readline(char *buf){
    int8_t len = vfs_fgets(current_script.handle, buf, current_script.filepointer);
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ECMD script: readline: %s\n", buf);
#endif // DEBUG_ECMD_SCRIPT
    if (len == -1 || len >= ECMD_INPUTBUF_LENGTH) {
      return 0;
    }
    current_script.filepointer += len + 1;
    current_script.linenumber++;
    return len;
}

int16_t
parse_cmd_goto(char *cmd, char *output, uint16_t len)
{ 
  uint8_t gotoline = 0;
  char line[ECMD_INPUTBUF_LENGTH];

  if (current_script.handle == NULL) {
      return ECMD_FINAL(snprintf_P(output, len, PSTR("no script")));
  }
  sscanf_P(cmd, PSTR("%i"), &gotoline);

#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ECMD script: current %i goto line %i\n", current_script.linenumber, gotoline);
#endif // DEBUG_ECMD_SCRIPT

  if (gotoline < current_script.linenumber) {
    vfs_fseek(current_script.handle, 0, SEEK_SET);
  }
  while ( current_script.linenumber != gotoline ) {
    readline(line);
  }
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_exit(char *cmd, char *output, uint16_t len)
{
  if (current_script.handle == NULL) {
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
  uint8_t lsize=0;
  uint8_t maxlines=100;
  uint8_t run=0;
  vfs_size_t filesize;
#ifdef DEBUG_ECMD_SCRIPT
  maxlines=10;
#endif // DEBUG_ECMD_SCRIPT

  sscanf_P(cmd, PSTR("%s"), &filename);  // should check for ".es" extention!
  current_script.handle = vfs_open(filename);

  if (current_script.handle == NULL)
    return ECMD_FINAL(1);
  
  filesize = vfs_size(current_script.handle);

#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ECMD script: start %s from %i bytes\n", filename, filesize);
#endif // DEBUG_ECMD_SCRIPT
  current_script.linenumber=0;
  current_script.filepointer=0;

  // open file as long it is open, we have not reached max lines and 
  // not the end of the file as we know it
  while ( (current_script.handle != NULL) && 
          (run++ < maxlines ) && 
          (filesize > current_script.filepointer ) ) {

    lsize = readline(line);
#ifdef DEBUG_ECMD_SCRIPT
      debug_printf("ECMD script: (linenr:%i, pos:%i, bufsize:%i)\n", current_script.linenumber, current_script.filepointer, lsize);
      debug_printf("ECMD script: exec: %s\n", line);
#endif // DEBUG_ECMD_SCRIPT
    if (lsize != 0) {
      ecmd_parse_command(line, output, len);
    }
  }
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ECMD script: end\n");
#endif // DEBUG_ECMD_SCRIPT

  parse_cmd_exit(cmd, output, len);

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_wait(char *cmd, char *output, uint16_t len)
{
  uint16_t delay;
  sscanf_P(cmd, PSTR("%i"), &delay);
#ifdef DEBUG_ECMD_SCRIPT
  debug_printf("wait %ims\n", delay);
#endif // DEBUG_ECMD_SCRIPT
  _delay_ms(delay);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_set(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  char value[10];
  sscanf_P(cmd, PSTR("%i %s"), &pos, &value);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES));
  }

  strcpy(vars[pos].value, value);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%%%i set to %s"), pos, vars[pos].value));
}

int16_t
parse_cmd_get(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES));
  }
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), vars[pos].value));
}

int16_t
parse_cmd_inc(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES));
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value + 1, vars[pos].value, 10);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_dec(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES));
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value - 1, vars[pos].value, 10);
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_if(char *cmd, char *output, uint16_t len)
{
  char cmpcmd[20];
  char comparator[3];
  char konst[10];
//  char cmd[]= "if ( whm != 00:01 ) then exit";
  uint8_t success = 0; // default false

  sscanf_P(cmd, PSTR("( %s %s %s ) then "), &cmpcmd, &comparator, &konst);
  char *ecmd = strstr_P(cmd, PSTR("then"));
  if (ecmd == NULL){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("cmd not found\n");
#endif // DEBUG_ECMD_SCRIPT
    return ECMD_FINAL(1);
  }
  ecmd+=5;
  
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("ecmd: %s\n", ecmd);
    debug_printf("cmpcmd: %s\n", cmpcmd);
    debug_printf("comparator: %s\n", comparator);
    debug_printf("konst: %s\n", konst);
#endif // DEBUG_ECMD_SCRIPT

  // if cmpcmd starts with % it is a variable
  if (cmpcmd[0]=='%') {
    uint8_t varpos = cmpcmd[1] - '0';
    // get variable and set it to output
    strcpy(output, vars[varpos].value );
  } else { // if not, it is a command
    // execute cmp! and check output
    if (!ecmd_parse_command(cmpcmd, output, len)) {
#ifdef DEBUG_ECMD_SCRIPT
      debug_printf("compare wrong\n");
#endif // DEBUG_ECMD_SCRIPT
      return ECMD_FINAL(1);
    }
  }
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("cmp '%s' %s '%s'\n", output, comparator, konst);
#endif // DEBUG_ECMD_SCRIPT

  // check comparator  
  if ( strcmp(comparator, STR_EQUALS) == 0 ){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " STR_EQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
    success = (strcmp(output, konst) == 0);
  } else if ( strcmp(comparator, STR_NOTEQUALS) == 0 ) {
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " STR_NOTEQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
    success = (strcmp(output, konst) != 0);
  } else {
    uint16_t outputvalue = atoi(output);
    uint16_t konstvalue = atoi(konst);
    debug_printf("cmp atoi: %i %s %i\n", outputvalue, comparator, konstvalue);
    if ( strcmp(comparator, OK) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " OK "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = outputvalue;
    } else if ( strcmp(comparator, NOT) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " NOT "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = ( outputvalue != 0 );
    } else if ( strcmp(comparator, EQUALS) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " EQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue == konstvalue);
    } else if ( strcmp(comparator, NOTEQUALS) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " NOTEQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue != konstvalue);
    } else if ( strcmp(comparator, GREATER) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " GREATER "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue > konstvalue);
    } else if ( strcmp(comparator, LOWER) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " LOWER "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue < konstvalue);
    } else if ( strcmp(comparator, GREATEREQUALS) == 0){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " GREATEREQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue >= konstvalue);
    } else if ( strcmp(comparator, LOWEREQUALS) == 0) {
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("try " LOWEREQUALS "\n");
#endif // DEBUG_ECMD_SCRIPT
      success = (outputvalue <= konstvalue);
    } else {
      debug_printf("unknown comparator: %s\n", comparator);
      return ECMD_FINAL(3);
    }
  }
  // if compare ok, execute command after then
  if (success){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("OK, do: %s\n", ecmd);
#endif // DEBUG_ECMD_SCRIPT
    if (ecmd_parse_command(ecmd, output, len)){
#ifdef DEBUG_ECMD_SCRIPT
      debug_printf("done: %s\n", output);
#endif // DEBUG_ECMD_SCRIPT
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"), output));
    }
    return ECMD_FINAL(2);
  }
#ifdef DEBUG_ECMD_SCRIPT
      debug_printf("success was: %i\n", success);
#endif // DEBUG_ECMD_SCRIPT
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


