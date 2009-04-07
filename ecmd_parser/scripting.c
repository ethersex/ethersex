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

#include <avr/pgmspace.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "../autoconf.h"
#include <util/delay.h>
#include <stdlib.h>
#include "../debug.h"
#include "ecmd.h"

#ifdef ECMD_SCRIPT_SUPPORT
#include "scripting.h"

int16_t
parse_cmd_goto(char *cmd, char *output, uint16_t len)
{
  sscanf_P(cmd, PSTR("%i"), &current_script.linenumber - 1);
  return 0;
}

int16_t
parse_cmd_exit(char *cmd, char *output, uint16_t len)
{
  current_script.file = NULL;
  current_script.linenumber=0;
  return 0;
}

int16_t
parse_cmd_call(char *cmd, char *output, uint16_t len)
{
  return snprintf_P(output, len, PSTR("not yet fully implemented"));

  sscanf_P(cmd, PSTR("%s"), &current_script.file);
  current_script.linenumber=0;

  // open file
  while (current_script.file!=NULL) {
    // read line
    char ecmd[50];
    // execute line
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("exec (%i): %s\n", current_script.linenumber, ecmd);
#endif // DEBUG_ECMD_SCRIPT
    ecmd_parse_command(ecmd, output, len);
    current_script.linenumber++;
  }
  return 0;
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
  return 0;
}

int16_t
parse_cmd_set(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  char value[10];
  sscanf_P(cmd, PSTR("%i %s"), &pos, &value);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES);
  }

  strcpy(vars[pos].value, value);
  return snprintf_P(output, len, PSTR("%%%i set to %s"), pos, vars[pos].value);
}

int16_t
parse_cmd_get(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES);
  }

  uint16_t value = atoi(vars[pos].value);
  return snprintf_P(output, len, PSTR("%s"), vars[pos].value);
}

int16_t
parse_cmd_inc(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES);
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value + 1, vars[pos].value, 10);
  return 0;
}

int16_t
parse_cmd_dec(char *cmd, char *output, uint16_t len)
{
  uint8_t pos;
  sscanf_P(cmd, PSTR("%i"), &pos);
  if (pos >= ECMD_SCRIPT_MAX_VARIABLES) {
    return snprintf_P(output, len, PSTR("max var exceed %i"), ECMD_SCRIPT_MAX_VARIABLES);
  }
  uint16_t value = atoi(vars[pos].value);
  itoa(value - 1, vars[pos].value, 10);
  return 0;
}

int16_t
parse_cmd_if(char *cmd, char *output, uint16_t len)
{
  char cmpcmd[20];
  char comparator[3];
  char konst[10];
//  char cmd[]= "if ( whm != 00:01 ) then exit";
  uint8_t comp = 0; // for ==

  sscanf_P(cmd, PSTR("( %s %s %s ) then "), &cmpcmd, &comparator, &konst);
  char *ecmd = strstr_P(cmd, PSTR("then"));
  if (ecmd == NULL){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("cmd not found\n");
#endif // DEBUG_ECMD_SCRIPT
    return 1;
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
      return 1;
    }
  }
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("Output: %s\n", output);
#endif // DEBUG_ECMD_SCRIPT

  // check comparator  
  if (strcmp(comparator, "==") != 0)
    comp = 1;

  // if compare ok, execute command after then
  if (strcmp(output, konst) == comp){
#ifdef DEBUG_ECMD_SCRIPT
    debug_printf("if successful, do: %s\n", ecmd);
#endif // DEBUG_ECMD_SCRIPT
    if (ecmd_parse_command(ecmd, output, len)){
#ifdef DEBUG_ECMD_SCRIPT
      debug_printf("done: %s\n", output);
#endif // DEBUG_ECMD_SCRIPT
      return snprintf_P(output, len, PSTR("%s"), output);
    }
    return 2;
  }

  return 1;
}

#endif  // ECMD_SCRIPT_SUPPORT
