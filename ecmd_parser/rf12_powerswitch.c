/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"
#include "../rfm12/rf12_powerswitch.h"
#include "ecmd.h"

#ifdef RF12_POWERSWITCH_SENDER_SUPPORT

int16_t
parse_cmd_rf12_powerswitch_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  //uint8_t command[17];
  uint16_t cnt;
  //uint8_t ret = sscanf_P (cmd, PSTR ("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u %u"),&command[0],&command[1],&command[2],&command[3],&command[4],&command[5],&command[6],&command[7],&command[8],&command[9],&command[10],&command[11],&command[12],&command[13],&command[14],&command[15], &cnt);
  
  uint16_t command;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u %u"), &command, &cnt);
  if (ret != 2)
    return -1;
  //rf_switchcode_send(command, cnt);
  rf12_powerswitch_send((uint8_t)command,(uint8_t)cnt);
  return 0;
}

int16_t
parse_cmd_rf12_powerswitch_code_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  uint16_t command[17];
  uint16_t cnt;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u %u"),&command[0],&command[1],&command[2],&command[3],&command[4],&command[5],&command[6],&command[7],&command[8],&command[9],&command[10],&command[11],&command[12],&command[13],&command[14],&command[15], &cnt);
  
  if (ret != 17)
    return -1;
  rf12_powerswitch_code_send(command, (uint8_t) cnt);
  return 0;
}

#endif  /* RF12_POWERSWITCH_SENDER_SUPPORT */

#ifdef RF12_POWERSWITCH_RECIVER_SUPPORT
int16_t
parse_cmd_rf12_powerswitch_recive(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
  rf12_powerswitch_reciver_init();
  return 0;
}
#endif  /* RF12_POWERSWITCH_RECIVER_SUPPORT */
