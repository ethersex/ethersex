/*
 * Copyright (c) 2012-13 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "protocols/ecmd/ecmd-base.h"

#include "rfm12.h"
#include "rfm12_fs20.h"
#include "rfm12_fs20_lib.h"
#include "rfm12_fs20_ecmd.h"

#ifdef RFM12_ASK_FS20_SUPPORT
typedef void (*fs20_func_t) (uint16_t, uint8_t, uint8_t, uint8_t);

static int16_t
parse_cmd_rfm12_internal(char *cmd, char *output, uint16_t len,
                         fs20_func_t func)
{
  uint16_t hc, addr, c, c2;

  int ret = sscanf_P(cmd, PSTR("%x %x %x %x"), &hc, &addr, &c, &c2);
  if (ret == 3 || ret == 4)
  {
    if (ret == 3)
    {
      c2 = 0;
    }

    func(hc, LO8(addr), LO8(c), LO8(c2));
    return ECMD_FINAL_OK;
  }

  return ECMD_ERR_PARSE_ERROR;

}

int16_t
parse_cmd_rfm12_fs20_send(char *cmd, char *output, uint16_t len)
{
  return parse_cmd_rfm12_internal(cmd, output, len, rfm12_fs20_send);
}

#endif

#ifdef RFM12_ASK_FHT_SUPPORT
int16_t
parse_cmd_rfm12_fht_send(char *cmd, char *output, uint16_t len)
{
  return parse_cmd_rfm12_internal(cmd, output, len, rfm12_fht_send);
}
#endif

int16_t
parse_cmd_rfm12_fs20_receive(char *cmd, char *output, uint16_t len)
{
  fs20_data_t fs20_data;
  if (rfm12_fs20_read(&fs20_data) == 0)
    return ECMD_FINAL_OK;

  int16_t len_out = 1;
  output[0] = fs20_data.datatype;
  if (fs20_data.nibble)
    fs20_data.count--;
  for (uint8_t i = 0; i < fs20_data.count; i++)
    len_out +=
      sprintf_P(&output[len_out], PSTR("%02" PRIX8), fs20_data.data[i]);
  if (fs20_data.nibble)
    len_out +=
      sprintf_P(&output[len_out], PSTR("%01" PRIX8),
                fs20_data.data[fs20_data.count] & 0xf);

  return ECMD_FINAL(len_out);
}

int16_t
parse_cmd_rfm12_fs20_setbandwidth(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t bandwidth;
  if (1 != sscanf_P(cmd, PSTR("%hhu"), &bandwidth))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(RFM12_MODUL_FS20);
  rfm12_setbandwidth(bandwidth, rfm12_modul->rfm12_gain,
                     rfm12_modul->rfm12_drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_fs20_setgain(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t gain;
  if (1 != sscanf_P(cmd, PSTR("%hhu"), &gain))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(RFM12_MODUL_FS20);
  rfm12_setbandwidth(rfm12_modul->rfm12_bandwidth, gain,
                     rfm12_modul->rfm12_drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_fs20_setdrssi(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t drssi;
  if (1 != sscanf_P(cmd, PSTR("%hhu"), &drssi))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(RFM12_MODUL_FS20);
  rfm12_setbandwidth(rfm12_modul->rfm12_bandwidth, rfm12_modul->rfm12_gain,
                     drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_fs20_setdebug(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t debug;
  if (1 != sscanf_P(cmd, PSTR("%hhx"), &debug))
    return ECMD_ERR_PARSE_ERROR;

  rx_report = debug;
  return ECMD_FINAL_OK;
}

/*
-- Ethersex META --
  block([[RFM12_FS20]])
  ecmd_ifdef(RFM12_ASK_FS20_SUPPORT)
    ecmd_feature(rfm12_fs20_send, "fs20 send", , housecode addr command data)
  ecmd_endif()
  ecmd_ifdef(RFM12_ASK_FHT_SUPPORT)
    ecmd_feature(rfm12_fht_send, "fht send", , housecode addr command data)
  ecmd_endif()
  ecmd_feature(rfm12_fs20_receive, "fs20 receive", , Receive FS20/FHT sequence and display it.)
  ecmd_feature(rfm12_fs20_setbandwidth, "fs20 setbandwidth", BW, Set receiver bandwidth to BW.)
  ecmd_feature(rfm12_fs20_setgain, "fs20 setgain", GAIN, Set preamplifier gain to GAIN.)
  ecmd_feature(rfm12_fs20_setdrssi, "fs20 setdrssi", DRSSI, Set the drssi to DRSSI.)
  ecmd_ifdef(DEBUG_ASK_FS20)
    ecmd_feature(rfm12_fs20_setdebug, "fs20 setdebug", DEBUG, Set debug to DEBUG.)
  ecmd_endif()
*/
