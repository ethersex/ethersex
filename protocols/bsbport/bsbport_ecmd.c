/*
 *
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) 2013 by Daniel Lindner <daniel.lindner@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include "config.h"
#include "core/bit-macros.h"
#include "core/util/fixedpoint.h"
#include "core/debug.h"
#include "protocols/bsbport/bsbport.h"
#include "protocols/bsbport/bsbport_helper.h"
#include "protocols/bsbport/bsbport_tx.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_bsbport_stats(char *cmd, char *output, uint16_t len)
{
  int16_t chars = snprintf_P(output, len,
                             PSTR
                             ("%uOK %uCRC %uLUV %uLOV %uDROP %uBF %uBNF %uRT"),
                             bsbport_rx_ok,
                             bsbport_rx_crcerror,
                             bsbport_rx_lenghtunder,
                             bsbport_rx_lenghtover,
                             bsbport_rx_dropped,
                             bsbport_rx_bufferfull,
                             bsbport_rx_net_bufferfull,
                             bsbport_eth_retransmit);
  return ECMD_FINAL(chars);
}

int16_t
parse_cmd_bsbport_list(char *cmd, char *output, uint16_t len)
{
  /*BUG prevention */
  len = len - 1;

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != 0x23)           /* indicator flag: real invocation:  0 */
  {
    cmd[0] = 0x23;              /* continuing call: 23 */
    cmd[1] = 0;                 /* counter for sensors in list */
  }
  uint8_t i = cmd[1];
  /* This is a special case: the while loop below printed a sensor which was
   * last in the list, so we still need to send an 'OK' after the sensor id */
  if (i >= BSBPORT_MESSAGE_BUFFER_LEN)
    return ECMD_FINAL_OK;

#ifdef DEBUG_BSBPORT_ECMD
  debug_printf("ECMD(%d) list MSG: %d ", len, i);
#endif

  int16_t ret = 0;
  ret =
    snprintf_P(output, len, PSTR("%02d %02x%02x%02x%02x %.1f %.1f %.1f %d"),
               i, bsbport_msg_buffer.msg[i].data[P1],
               bsbport_msg_buffer.msg[i].data[P2],
               bsbport_msg_buffer.msg[i].data[P3],
               bsbport_msg_buffer.msg[i].data[P4],
               bsbport_msg_buffer.msg[i].value_temp,
               bsbport_msg_buffer.msg[i].value_FP1,
               bsbport_msg_buffer.msg[i].value_FP5,
               bsbport_msg_buffer.msg[i].value_raw);
  i++;

#ifdef DEBUG_BSBPORT_ECMD
  debug_printf("ECMD list write %d bytes", ret);
#endif

  cmd[1] = i;
  return ECMD_AGAIN(ret);
}

int16_t
parse_cmd_bsbport_get(char *cmd, char *output, uint16_t len)
{
  /*BUG prevention */
  len = len - 1;

  uint8_t p1 = 0;
  uint8_t p2 = 0;
  uint8_t p3 = 0;
  uint8_t p4 = 0;
  uint8_t src = 0;
  int16_t ret = 0;
  char type[5];

  //Trim Leading spaces
  while (cmd[0] == ' ')
  {
    cmd++;
  }

  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %s"), &p1, &p2, &p3, &p4,
             &src, type);

#ifdef DEBUG_BSBPORT_ECMD
  debug_printf("ECMD(%d) get MSG ARGS:%d %3s %02x %02x %02x %02x ", len, ret,
               type, p1, p2, p3, p4);
#endif

  if (ret == 6)
  {
    for (uint8_t i = 0; i < BSBPORT_MESSAGE_BUFFER_LEN; i++)
    {
      if (bsbport_msg_buffer.msg[i].len != 0
          && bsbport_msg_buffer.msg[i].data[P1] == p1
          && bsbport_msg_buffer.msg[i].data[P2] == p2
          && bsbport_msg_buffer.msg[i].data[P3] == p3
          && bsbport_msg_buffer.msg[i].data[P4] == p4
          && (bsbport_msg_buffer.msg[i].data[SRC] & 0x0F) == src)
      {
#ifdef DEBUG_BSBPORT_ECMD
        debug_printf("ECMD get MSG found at: %02d ", i);
#endif
        if (strcmp_P(type, PSTR("RAW")) == 0)
        {
          ret =
            snprintf_P(output, len, PSTR("%u"),
                       bsbport_msg_buffer.msg[i].value_raw);
        }
        else if (strcmp_P(type, PSTR("STAT")) == 0)
        {
          ret =
            snprintf_P(output, len, PSTR("%u"),
                       (uint8_t)(bsbport_msg_buffer.msg[i].value_raw >> 8));
        }
        else if (strcmp_P(type, PSTR("TMP")) == 0)
        {
          ret =
            snprintf_P(output, len, PSTR("%.1f"),
                       bsbport_msg_buffer.msg[i].value_temp);
        }
        else if (strcmp_P(type, PSTR("FP1")) == 0)
        {
          ret =
            snprintf_P(output, len, PSTR("%.1f"),
                       bsbport_msg_buffer.msg[i].value_FP1);
        }
        else if (strcmp_P(type, PSTR("FP5")) == 0)
        {
          ret =
            snprintf_P(output, len, PSTR("%.1f"),
                       bsbport_msg_buffer.msg[i].value_FP5);
        }
        else
          return ECMD_ERR_PARSE_ERROR;

#ifdef DEBUG_BSBPORT_ECMD
        debug_printf("ECMD get write %d bytes", ret);
#endif
        return ECMD_FINAL(ret);
      }
    }
    ret = snprintf_P(output, len, PSTR("message not in list!"));
    return ECMD_FINAL(ret);
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_bsbport_query(char *cmd, char *output, uint16_t len)
{
  uint8_t p1 = 0;
  uint8_t p2 = 0;
  uint8_t p3 = 0;
  uint8_t p4 = 0;
  uint8_t dest = 0;
  int16_t ret = 0;
  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi"), &p1, &p2, &p3, &p4,
             &dest);
  if (ret == 4 || ret == 5)
  {
    if (bsbport_query(p1, p2, p3, p4, dest))
      return ECMD_FINAL_OK;
    else
      return ECMD_FINAL(snprintf_P(output, len, PSTR("txbuffer is full!")));
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_bsbport_set(char *cmd, char *output, uint16_t len)
{
  float val = 0;
  uint8_t p1 = 0;
  uint8_t p2 = 0;
  uint8_t p3 = 0;
  uint8_t p4 = 0;
  uint8_t dest = 0;
  int16_t ret = 0;
  char type[4];
  char strvalue[9];
  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %3s %s"), &p1, &p2, &p3, &p4,
             &dest, type, strvalue);
  val = strtod(strvalue, NULL);
#ifdef DEBUG_BSBPORT_ECMD
  debug_printf("ECMD(%d) set MSG ARGS:%d %02x %02x %02x %02x %3s %s %f", len,
               ret, p1, p2, p3, p4, type, strvalue, val);
#endif
  if (ret == 6 || ret == 7)
  {
    uint8_t data[3];
    uint8_t datalen = 3;
    if (strcmp_P(type, PSTR("RAW")) == 0)
    {
      data[0] = 0x01;
      data[1] = (uint8_t) ((uint16_t) val >> 8);
      data[2] = (uint8_t) (0x00FF & (uint16_t) val);
      datalen = 3;
    }
    else if (strcmp_P(type, PSTR("SEL")) == 0)
    {
      data[0] = 0x01;
      data[1] = (uint8_t) (val);
      datalen = 2;
    }
    else if (strcmp_P(type, PSTR("TMP")) == 0)
    {
      bsbport_ConvertTempToData(val, &data[1]);
      data[0] = 0x01;
      datalen = 3;
    }
    else if (strcmp_P(type, PSTR("FP1")) == 0)
    {
      int16_t tmp = 0;
      data[0] = 0x01;
      tmp = val * 10;
      memcpy(&data[1], &tmp, 2);
      datalen = 3;
    }
    else if (strcmp_P(type, PSTR("FP5")) == 0)
    {
      int16_t tmp = 0;
      data[0] = 0x01;
      tmp = val * 2;
      memcpy(&data[1], &tmp, 2);
      datalen = 3;
    }
    else
      return ECMD_FINAL(snprintf_P(output, len, PSTR("type unknown")));

#ifdef DEBUG_BSBPORT_ECMD
    debug_printf("ECMD set parsed data: %02x %02x %02x %02d ", data[0],
                 data[1], data[2], datalen);
#endif

    if (bsbport_set(p1, p2, p3, p4, dest, data, datalen))
      return ECMD_FINAL_OK;
    else
      return ECMD_FINAL(snprintf_P(output, len, PSTR("txbuffer is full!")));
  }
  else
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("param count at: %d"), ret));
}


/*
  -- Ethersex META --
  block([[BSBPORT]] commands)
  ecmd_feature(bsbport_stats, "bsbport stats",, Report statistic counters OK/CRC/Lenght/Frame/Overflow/Parity/Buffer/BufferNet/Retransmit)
  ecmd_feature(bsbport_list, "bsbport list",, List all messages currently in buffer)
  ecmd_feature(bsbport_get, "bsbport get",P1 P2 P3 P4 SRC TYPE, Show specific message currently in buffer format value as TYPE, type is one of RAW,STAT,TMP,FP1,FP5)
  ecmd_feature(bsbport_set, "bsbport set",P1 P2 P3 P4 DEST TYPE VALUE, Send Message to set value, type is one of RAW,SEL,TMP,FP1,FP5)
  ecmd_feature(bsbport_query, "bsbport query",P1 P2 P3 P4 [DEST], Send Message to query for a value, DEST is optional defaults to 0 )
*/
