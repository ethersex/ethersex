/*
 *
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include "core/util/string_parsing.h"
#include "core/debug.h"
#include "protocols/bsbport/bsbport.h"
#include "protocols/bsbport/bsbport_helper.h"
#include "protocols/bsbport/bsbport_tx.h"
#include "protocols/ecmd/ecmd-base.h"

#ifdef DEBUG_BSBPORT_ECMD
#define BSBPORT_DEBUG(s, ...) debug_printf("BSB " s "\n", ## __VA_ARGS__);
#else
#define BSBPORT_DEBUG(a...) do {} while(0)
#endif

int16_t
parse_cmd_bsbport_stats(const char *const cmd, char *output,
                        const uint16_t len)
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
parse_cmd_bsbport_list(char *const cmd, char *output, const uint16_t len)
{

  /* trick: use bytes on cmd as "connection specific static variables" */
  if (cmd[0] != 0x23)           /* indicator flag: real invocation:  0 */
  {
    cmd[0] = 0x23;              /* continuing call: 23 */
    cmd[1] = 0;                 /* counter for message in list */
  }
  uint8_t i = cmd[1];
  /* This is a special case: the while loop below printed a message which was
   * last in the list or the next message is empty, so we still need to send an 'OK' after the message */
  if (i >= BSBPORT_MESSAGE_BUFFER_LEN
      || (bsbport_msg_buffer.msg[i].p.raw == 0))
    return ECMD_FINAL_OK;

  BSBPORT_DEBUG("ECMD(%d) list MSG: %d ", len, i);

  int16_t ret = 0;
  ret =
    snprintf_P(output, len, PSTR("%d\t%02x%02x%02x%02x\t%d"),
               i,
               bsbport_msg_buffer.msg[i].p.data.p1,
               bsbport_msg_buffer.msg[i].p.data.p2,
               bsbport_msg_buffer.msg[i].p.data.p3,
               bsbport_msg_buffer.msg[i].p.data.p4, bsbport_msg_buffer.msg[i].value);
  if (len - ret > 0)
    output[ret++] = '\t';
  ret +=
    itoa_fixedpoint(((int32_t) bsbport_msg_buffer.msg[i].value * 100) /
                    64, 2, output + ret, len - ret);
  if (len - ret > 0)
    output[ret++] = '\t';
  ret +=
    itoa_fixedpoint(bsbport_msg_buffer.msg[i].value, 1, output + ret,
                    len - ret);
  if (len - ret > 0)
    output[ret++] = '\t';
  ret +=
    itoa_fixedpoint(bsbport_msg_buffer.msg[i].value * 10 / 2, 1,
                    output + ret, len - ret);

  i++;

  BSBPORT_DEBUG("ECMD list write %d bytes", ret);

  cmd[1] = i;
  return ECMD_AGAIN(ret);
}

int16_t
parse_cmd_bsbport_get(const char *cmd, char *output, const uint16_t len)
{

  uint8_t p1;
  uint8_t p2;
  uint8_t p3;
  uint8_t p4;
  uint8_t src;
  int16_t ret;
  char type[4];

  //Trim Leading spaces
  while (cmd[0] == ' ')
  {
    cmd++;
  }

  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %3s"), &p1, &p2, &p3, &p4,
             &src, type);

  BSBPORT_DEBUG("ECMD(%d) get MSG ARGS:%d %3s %02x %02x %02x %02x ", len, ret,
               type, p1, p2, p3, p4);

  if (ret == 6)
  {
    for (uint8_t i = 0; i < BSBPORT_MESSAGE_BUFFER_LEN; i++)
    {
      if (bsbport_msg_buffer.msg[i].data_length != 0
          && bsbport_msg_buffer.msg[i].p.data.p1 == p1
          && bsbport_msg_buffer.msg[i].p.data.p2 == p2
          && bsbport_msg_buffer.msg[i].p.data.p3 == p3
          && bsbport_msg_buffer.msg[i].p.data.p4 == p4
          && (bsbport_msg_buffer.msg[i].src & 0x0F) == src)
      {
        BSBPORT_DEBUG("ECMD get MSG found at: %02d ", i);

        if (type[0] == 'R') //get RAW value
        {
          ret =
            snprintf_P(output, len, PSTR("%u"),
                       bsbport_msg_buffer.msg[i].value);
        }
        else if (type[0] == 'S') //get SELECT/SEL value
        {
          ret =
            snprintf_P(output, len, PSTR("%u"),
                       HI8(bsbport_msg_buffer.msg[i].value));
        }
        else if (type[0] == 'T') //get TEMP/TMP value
        {
          ret =
            itoa_fixedpoint(((int32_t) bsbport_msg_buffer.msg[i].value *
                             100) / 64, 2, output, len);
        }
        else if (type[0] == 'F') //get FP1/FP5 value
        {
          if (type[2] == '1') //set FP1 value
          {
            ret =
              itoa_fixedpoint(bsbport_msg_buffer.msg[i].value, 1, output, len);
          }
          else if (type[2] == '5') //get FP5 value
          {
            ret =
              itoa_fixedpoint(bsbport_msg_buffer.msg[i].value * 10 / 2, 1,
                              output, len);
          }
        }
        else
          return ECMD_ERR_PARSE_ERROR;

        BSBPORT_DEBUG("ECMD get write %d bytes", ret);

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
parse_cmd_bsbport_query(const char *const cmd, char *output,
                        const uint16_t len)
{
  uint8_t p1;
  uint8_t p2;
  uint8_t p3;
  uint8_t p4;
  uint8_t dest = 0;
  uint8_t ret;
  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi"), &p1, &p2, &p3, &p4,
             &dest);
  if (ret == 4 || ret == 5)
  {
    if (bsbport_query(p1, p2, p3, p4, dest))
      return ECMD_FINAL_OK;
    else
      return ECMD_ERR_WRITE_ERROR;
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}

int16_t
parse_cmd_bsbport_set(const char *const cmd, char *output, const uint16_t len)
{
  int16_t fp_val = 0;
  uint16_t raw_val = 0;
  uint8_t p1;
  uint8_t p2;
  uint8_t p3;
  uint8_t p4;
  uint8_t dest;
  uint8_t ret;
  char type[4];
  char strvalue[9];
  ret =
    sscanf_P(cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %3s %s"), &p1, &p2, &p3, &p4,
             &dest, type, strvalue);
  sscanf_P(strvalue, PSTR("%i"), &raw_val);
  next_int16_fp(strvalue, &fp_val, 1);

  BSBPORT_DEBUG("ECMD(%d) set MSG ARGS:%d %02x %02x %02x %02x %3s %s %d %u\n",
               len, ret, p1, p2, p3, p4, type, strvalue, fp_val, raw_val);

  if (ret == 7)
  {
    uint8_t data[3];
    uint8_t datalen = 3;
    data[0] = 0x01;
    if (type[0] == 'R') //set RAW value
    {
      data[1] = HI8(raw_val);
      data[2] = LO8(raw_val);
    }
    else if (type[0] == 'S') //set SELECT/SEL value
    {
      data[1] = LO8(raw_val);
      datalen = 2;
    }
    else if (type[0] == 'T') //set TEMP/TMP value
    {
      int16_t tmp;
      tmp = ((int32_t) fp_val * 64) / 10;
      data[1] = HI8(tmp);
      data[2] = LO8(tmp);
    }
    else if (type[0] == 'F') //set FP1/FP5 value
    {
      if (type[2] == '1') //set FP1 value
      {
        data[1] = HI8(fp_val);
        data[2] = LO8(fp_val);
      }
      else if (type[2] == '5') //set FP1 value
      {
        int16_t tmp;
        tmp = fp_val * 2 / 10;
        data[1] = HI8(tmp);
        data[2] = LO8(tmp);
      }
    }
    else
      return ECMD_ERR_PARSE_ERROR;

    BSBPORT_DEBUG("ECMD set parsed data: %02x %02x %02x %02d ", data[0],
                 data[1], data[2], datalen);

    if (bsbport_set(p1, p2, p3, p4, dest, data, datalen))
      return ECMD_FINAL_OK;
    else
      return ECMD_ERR_WRITE_ERROR;
  }
  else
    return ECMD_ERR_PARSE_ERROR;
}


/*
  -- Ethersex META --
  block([[BSBPORT]] commands)
  ecmd_feature(bsbport_stats, "bsbport stats",, Report statistic counters OK/CRC/Lenght under/Lenght over/Droped/Buffer/BufferNet/Retransmit)
  ecmd_feature(bsbport_list, "bsbport list",, List all messages currently in buffer output in RAW TMP FP1 FP5)
  ecmd_feature(bsbport_get, "bsbport get",P1 P2 P3 P4 SRC TYPE, Show specific message currently in buffer format value as TYPE type is one of RAW STA TMP FP1 FP5)
  ecmd_feature(bsbport_set, "bsbport set",P1 P2 P3 P4 DEST TYPE VALUE, Send Message to set value type is one of RAW SEL TMP FP1 FP5)
  ecmd_feature(bsbport_query, "bsbport query",P1 P2 P3 P4 [DEST], Send Message to query for a value DEST is optional defaults to 0 )
*/
