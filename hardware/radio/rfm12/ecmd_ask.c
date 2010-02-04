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

#include "config.h"
#include "core/debug.h"
#include "hardware/radio/rfm12/rfm12_ask.h"

#include "protocols/ecmd/ecmd-base.h"


#ifdef RFM12_ASK_SENDER_SUPPORT

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
  uint8_t ret = sscanf_P (cmd, PSTR ("%hhu,%hhu,%hhu %hhu,%hhu %hhu %hhu"),&housecode[0], &housecode[1], &housecode[2], &command[0], &command[1], &delay, &cnt);
  if (ret < 5)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_tevion_send(housecode, command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif // RFM12_ASK_TEVION_SUPPORT

#ifdef RFM12_ASK_2272_SUPPORT
int16_t
parse_cmd_rfm12_ask_2272_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 10;
  uint8_t ret = sscanf_P (cmd, PSTR ("%hhu,%hhu,%hhu %hhu %hhu"),&(command[0]), &(command[1]), &(command[2]), &delay, &cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_2272_1527_switch(T_2272);
  rfm12_ask_2272_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif // RFM12_ASK_2272_SUPPORT

#ifdef RFM12_ASK_1527_SUPPORT
int16_t
parse_cmd_rfm12_ask_1527_send(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t command[3];
  uint8_t delay = 74;
  uint8_t cnt = 10;
  uint8_t ret = sscanf_P (cmd, PSTR ("%hhu,%hhu,%hhu %hhu %hhu"),&(command[0]), &(command[1]), &(command[2]), &delay, &cnt);
  if (ret < 3)
    return ECMD_ERR_PARSE_ERROR;

  rfm12_ask_2272_1527_switch(T_1527);
  rfm12_ask_2272_send(command, delay, cnt);
  return ECMD_FINAL_OK;
}
#endif // RFM12_ASK_1527_SUPPORT

#endif  /* RFM12_ASK_SENDER_SUPPORT */

#ifdef RFM12_ASK_EXTERNAL_FILTER_SUPPORT
int16_t
    parse_cmd_rfm12_ask_external_filter(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  uint8_t flag;
  uint8_t ret = sscanf_P (cmd, PSTR ("%hhu"),&flag);
  if (ret != 1)
    rfm12_ask_external_filter_deinit();
  else
    rfm12_ask_external_filter_init();

  return ECMD_FINAL_OK;
}

#ifdef RFM12_ASK_SENSING_SUPPORT
int16_t
parse_cmd_rfm12_ask_sense (char *cmd, char *output, uint16_t len)
{
  (void) cmd;
  (void) output;
  (void) len;

  rfm12_ask_sense_start ();
  return ECMD_FINAL_OK;
}
#endif  /* RFM12_ASK_SENSING_SUPPORT */
#endif  /* RFM12_ASK_EXTERNAL_FILTER_SUPPORT */
