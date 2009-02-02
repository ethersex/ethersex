/**********************************************************
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * @author      Gregor B.
 * @author      Dirk Pannenbecker
 * @author      Guido Pannenbecker
 * @author      Stefan Riepenhausen
 * @date        01.02.2009

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/
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
parse_cmd_rf12_powerswitch_tevion_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint16_t housecode_16_1,housecode_16_2,housecode_16_3;
  uint16_t command_16_1,command_16_2;
  uint16_t delay;
  uint16_t cnt;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u,%u,%u %u,%u %u %u"),&housecode_16_1, &housecode_16_2, &housecode_16_3, &command_16_1, &command_16_2, &delay, &cnt);
  if (ret != 7)
    return -1;

  uint8_t housecode[3]={housecode_16_1,housecode_16_2,housecode_16_3};
  uint8_t command[2]={command_16_1,command_16_2};

  rf12_powerswitch_tevion_send(housecode, command, delay, cnt);
  return 0;
}

int16_t
    parse_cmd_rf12_powerswitch_2272_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint16_t command_16_0,command_16_1,command_16_2;
  uint16_t delay;
  uint16_t cnt;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u,%u,%u %u %u"),&command_16_0, &command_16_1, &command_16_2, &delay, &cnt);
  if (ret != 5)
    return -1;

  uint8_t command[3]={command_16_0,command_16_1,command_16_2};

  rf12_powerswitch_2272_send(command, delay, cnt);
  return 0;
}

#endif  /* RF12_POWERSWITCH_SENDER_SUPPORT */

#ifdef RF12_POWERSWITCH_RECEIVER_SUPPORT
int16_t
    parse_cmd_rf12_powerswitch_receive(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  uint16_t flag;
  uint8_t ret = sscanf_P (cmd, PSTR ("%u"),&flag);
  if (ret != 1)
    rf12_powerswitch_receiver_deinit();
  else
    rf12_powerswitch_receiver_init();

  return 0;
}
#endif  /* RF12_POWERSWITCH_RECEIVER_SUPPORT */
