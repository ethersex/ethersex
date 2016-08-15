/*
 * Copyright (c) Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2013 Erik Kunze <ethersexerik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "config.h"
#include "version.h"
#include "core/debug.h"
#include "core/heartbeat.h"
#include "protocols/uip/uip.h"
#include "core/eeprom.h"
#include "control6/control6.h"
#ifdef ECMD_LOG_VIA_SYSLOG
#include "protocols/syslog/syslog.h"
#endif
#include "protocols/ecmd/aliascmd.h"
#include "protocols/ecmd/parser.h"

#include "protocols/ecmd/ecmd-base.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"


#define xstr(s) str(s)
#define str(s) #s

int16_t
ecmd_parse_command(char *cmd, char *output, uint16_t len)
{
#ifdef DEBUG_ECMD
  debug_printf("called ecmd_parse_command %s\n", cmd);
#endif
#ifdef ECMD_LOG_VIA_SYSLOG
  if (0 == strchr(cmd, ECMD_STATE_MAGIC))
    syslog_sendf_P(PSTR("ecmd: %s"), cmd);
#endif

#ifdef ECMD_REMOVE_BACKSPACE_SUPPORT
  uint8_t i = 0;
  while (cmd[i] != '\0' && cmd[i] != ECMD_STATE_MAGIC &&
         i < ECMD_OUTPUTBUF_LENGTH)
  {                             // search until end of string
    if (cmd[i] == '\b')
    {                           // check cmd for backspaces
      uint16_t cmdlen = strlen(cmd + i);
      memmove(cmd + i - 1, cmd + i + 1, cmdlen);        // we found a backspace, so we move all chars backwards
      i--;                      // and decrement char counter
    }
    else
    {
      i++;                      // goto char
    }
  }
#endif /* ECMD_REMOVE_BACKSPACE_SUPPORT */

#ifdef ALIASCMD_SUPPORT
  if (cmd[0] == '$')
  {                             // alias command names start with $
#ifdef DEBUG_ECMD
    debug_printf("try alias\n");
#endif
    if (aliascmd_decode(cmd) == NULL)
    {
      // command not found in alias list
#ifdef DEBUG_ECMD
      debug_printf("Alias failed\n");
#endif
    }
    else
    {
#ifdef DEBUG_ECMD
      debug_printf("new command: %s\n", cmd);
#endif
    }
  }
#endif /* ALIASCMD_SUPPORT */

  if (strlen(cmd) < 2)
  {
#ifdef DEBUG_ECMD
    debug_printf("cmd is too short\n");
#endif
    return 0;
  }

  int ret = -1;

  char *text = NULL;
  int16_t(*func) (char *, char *, uint16_t) = NULL;
  uint8_t pos = 0;

  while (1)
  {
    /* load pointer to text */
    text = (char *) pgm_read_word(&ecmd_cmds[pos].name);

#ifdef DEBUG_ECMD
    debug_printf("loaded text addres %p: \n", text);
#endif

    /* return if we reached the end of the array */
    if (text == NULL)
      break;

#ifdef DEBUG_ECMD
    debug_printf("text is: \"%S\"\n", text);
#endif

    /* else compare texts */
    if (memcmp_P(cmd, text, strlen_P(text)) == 0)
    {
#ifdef DEBUG_ECMD
      debug_printf("found match\n");
#endif
      cmd += strlen_P(text);
      func = (void *) pgm_read_word(&ecmd_cmds[pos].func);
      break;
    }

    pos++;
  }

#ifdef DEBUG_ECMD
  debug_printf("rest cmd: \"%s\"\n", cmd);
#endif

  ACTIVITY_LED_ECMD;

  if (func != NULL)
    ret = func(cmd, output, len);

  if (output != NULL)
  {
    if (ret == -1)
    {
      memcpy_P(output, PSTR("parse error"), 11);
      ret = 11;
    }
    else if (ret == 0)
    {
      output[0] = 'O';
      output[1] = 'K';
      ret = 2;
    }
  }

  return ret;
}

#ifdef FREE_SUPPORT
extern char *__brkval;
extern unsigned char __heap_start;

int16_t
parse_cmd_free(char *cmd, char *output, uint16_t len)
{
  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {                             /* indicator flag: real invocation:  0 */
    cmd[0] = ECMD_STATE_MAGIC;  /*                 continuing call: 23 */
    cmd[1] = 0;                 /* counter for output lines */
  }

  /* Docu March 2009: http://www.nongnu.org/avr-libc/user-manual/malloc.html
   * Stack size: RAMEND-SP
   * Heap size: __brkval-__heap_start
   * Space between stack and heap: SP-__brkval
   * Caution: __brkval is 0 when malloc was not called yet (use __heap_start instead)
   * 
   * Size of network packet frames is stored in NET_MAX_FRAME_LENGTH
   */

  size_t f =
    (size_t) (__brkval ? (size_t) __brkval : (size_t) & __heap_start);

  /* we want an output like this:
   * free: 16234/32768
   * heap: 10234
   * net: 500
   */
  switch (cmd[1]++)
  {
    case 0:
      return ECMD_AGAIN(snprintf_P(output, len,
                                   PSTR("free: %u/%u"),
                                   SP - f, RAM_SIZE));
    case 1:
      return ECMD_AGAIN(snprintf_P(output, len,
                                   PSTR("heap: %u"),
                                   f - (size_t) & __heap_start));
  }
  return ECMD_FINAL(snprintf_P(output, len,
                               PSTR("net: " xstr(NET_MAX_FRAME_LENGTH))));
}
#endif /* FREE_SUPPORT */

#ifndef TEENSY_SUPPORT
int16_t
parse_cmd_version(char *cmd, char *output, uint16_t len)
{
  (void) cmd;

  strncpy_P(output, pstr_E6_VERSION_STRING_LONG, len);
  output[len - 1] = '\0';

  return ECMD_FINAL(strlen(output));
}

int16_t
parse_cmd_help(char *cmd, char *output, uint16_t len)
{
  uint16_t help_len;
  (void) help_len;

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != ECMD_STATE_MAGIC)
  {                             /* indicator flag: real invocation:  0 */
    cmd[0] = ECMD_STATE_MAGIC;  /*                 continuing call: 23 */
    cmd[1] = 0;                 /* counter for output lines */
  }

  char *text = (char *) pgm_read_word(&ecmd_cmds[(uint8_t) cmd[1]++].name);
  help_len = strlen_P(text);
  len = len < help_len ? len : help_len;
  memcpy_P(output, text, len);

  text = (char *) pgm_read_word(&ecmd_cmds[(uint8_t) cmd[1]].name);
  return text ? ECMD_AGAIN(len) : ECMD_FINAL(len);
}
#endif /* TEENSY_SUPPORT */

#ifdef EEPROM_SUPPORT
int16_t
parse_cmd_eeprom_reinit(char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  eeprom_init();
  return ECMD_FINAL_OK;
}
#endif /* EEPROM_SUPPORT */
