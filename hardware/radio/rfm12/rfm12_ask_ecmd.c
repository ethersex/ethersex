/*
 * Copyright (c) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) Gregor B.
 * Copyright (c) Dirk Pannenbecker
 * Copyright (c) Guido Pannenbecker
 * Copyright (c) Stefan Riepenhausen
 * Copyright (c) 2012-13 Erik Kunze <ethersex@erik-kunze.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#include "rfm12.h"
#include "rfm12_ask.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef TEENSY_SUPPORT
#ifdef RFM12_ASK_2272_SUPPORT
uint8_t
getIntFromString(char *cmd)
{
  uint8_t ptr = 0;
  char str[3];

  while ((cmd[ptr] >= '0') && (cmd[ptr] <= '9'))
  {                             /* count digits */
    ptr++;
  }
  strncpy(str, cmd, ptr);
  return atoi(str);
}
#endif /* RFM12_ASK_2272_SUPPORT */
#endif /* TEENSY_SUPPORT */

#ifdef RFM12_ASK_TEVION_SUPPORT
int16_t
parse_cmd_rfm12_ask_tevion_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t housecode[3];
  uint8_t command[2];
  uint8_t delay = 99;
  uint8_t cnt = 4;
  int ret = sscanf_P(cmd, PSTR("%hhu,%hhu,%hhu %hhu,%hhu %hhu %hhu"),
                     &housecode[0], &housecode[1], &housecode[2],
                     &command[0], &command[1], &delay, &cnt);
  if (ret < 5)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_tevion_send(housecode, command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_TEVION_SUPPORT */

#ifdef RFM12_ASK_INTERTECHNO_SUPPORT
int16_t
parse_cmd_rfm12_ask_intertechno_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t family;
  uint8_t group;
  uint8_t device;
  uint8_t command;
  int ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"), &family, &group,
                     &device, &command);
  if (ret < 4)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_intertechno_send(family, group, device, command);
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_INTERTECHNO_SUPPORT */

#ifdef RFM12_ASK_2272_SUPPORT
int16_t
parse_cmd_rfm12_ask_2272_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 10;
#ifdef TEENSY_SUPPORT
  while (*cmd == ' ')
    cmd++;
  command[0] = getIntFromString(cmd);
  while (*cmd != ',')
    cmd++;
  cmd++;
  command[1] = getIntFromString(cmd);
  while (*cmd != ',')
    cmd++;
  cmd++;
  command[2] = getIntFromString(cmd);
  while (*cmd != ' ')
    cmd++;
  cmd++;
  delay = getIntFromString(cmd);
  while (*cmd != ' ')
    cmd++;
  cmd++;
  cnt = getIntFromString(cmd);
  int ret = 5;
#else
  int ret = sscanf_P(cmd, PSTR("%hhu,%hhu,%hhu %hhu %hhu"), &(command[0]),
                     &(command[1]), &(command[2]), &delay, &cnt);
#endif
  RFM12_DEBUG("ps cmd %u,%u,%u d %u s %u", command[0], command[1],
              command[2], delay, cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_2272_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_2272_SUPPORT */

#ifdef RFM12_ASK_1527_SUPPORT
int16_t
parse_cmd_rfm12_ask_1527_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 10;
  int ret = sscanf_P(cmd, PSTR("%hhu,%hhu,%hhu %hhu %hhu"), &(command[0]),
                     &(command[1]), &(command[2]), &delay, &cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_1527_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_1527_SUPPORT */

#ifdef RFM12_ASK_OASEFMMASTER_SUPPORT
int16_t
parse_cmd_rfm12_ask_oase_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 20;
  int ret = sscanf_P(cmd, PSTR("%hhu,%hhu,%hhu %hhu %hhu"), &(command[0]),
                     &(command[1]), &(command[2]), &delay, &cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_oase_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_OASEFMMASTER_SUPPORT */

#ifdef RFM12_ASK_EXTERNAL_FILTER_SUPPORT
int16_t
parse_cmd_rfm12_ask_external_filter(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t flag;
  int ret = sscanf_P(cmd, PSTR("%hhu"), &flag);
  if (ret == 1 && flag == 1)
    rfm12_ask_external_filter_init();
  else
    rfm12_ask_external_filter_deinit();

  return ECMD_FINAL_OK;
}

#ifdef RFM12_ASK_SENSING_SUPPORT
int16_t
parse_cmd_rfm12_ask_sense(char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  rfm12_ask_sense_start();
  return ECMD_FINAL_OK;
}
#endif /* RFM12_ASK_SENSING_SUPPORT */
#endif /* RFM12_ASK_EXTERNAL_FILTER_SUPPORT */

/*
  -- Ethersex META --
  block([[RFM12_ASK]])
  ecmd_ifdef(RFM12_ASK_TEVION_SUPPORT)
    ecmd_feature(rfm12_ask_tevion_send, "rfm12 tevion", , housecode command delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_2272_SUPPORT)
    ecmd_feature(rfm12_ask_2272_send, "rfm12 2272", , housecodeCommand delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_1527_SUPPORT)
    ecmd_feature(rfm12_ask_1527_send, "rfm12 1527", , housecodeCommand delay cnt)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_INTERTECHNO_SUPPORT)
    ecmd_feature(rfm12_ask_intertechno_send, "rfm12 intertechno", , family group device command)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_OASEFMMASTER_SUPPORT)
    ecmd_feature(rfm12_ask_oase_send, "rfm12 oase", , family group device command)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_EXTERNAL_FILTER_SUPPORT)
    ecmd_feature(rfm12_ask_external_filter, "rfm12 external filter",[1], Enable ext. filter pin if argument is present (disable otherwise))
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_SENSING_SUPPORT)
    ecmd_feature(rfm12_ask_sense, "rfm12 ask sense",, Trigger (Tevion) ASK sensing.  Enable ext. filter pin before!)
  ecmd_endif()
*/
