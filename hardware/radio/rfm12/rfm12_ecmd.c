/*
 * Copyright (c) 2008 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#include "protocols/ecmd/ecmd-base.h"

#include "rfm12.h"
#include "rfm12_net.h"
#include "rfm12_ecmd.h"

int16_t
parse_cmd_rfm12_status(char *cmd, char *output, uint16_t len)
{
  uint8_t module;

  if ((1 != sscanf_P(cmd, PSTR("%hhu"), &module)) ||
      (module >= RFM12_MODULE_COUNT))
    return ECMD_ERR_PARSE_ERROR;

  uint16_t s;
  rfm12_prologue(module);
  s = rfm12_get_status();
  rfm12_epilogue();

  return ECMD_FINAL(snprintf_P(output, len, PSTR("rfm12 status: %04x"), s));
}

int16_t
parse_cmd_rfm12_setbandwidth(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t module;
  uint8_t bandwidth;
  if ((2 != sscanf_P(cmd, PSTR("%hhu %hhu"), &module, &bandwidth)) ||
      (module >= RFM12_MODULE_COUNT))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(module);
  rfm12_setbandwidth(bandwidth, rfm12_modul->rfm12_gain,
                     rfm12_modul->rfm12_drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_setgain(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t module;
  uint8_t gain;
  if ((2 != sscanf_P(cmd, PSTR("%hhu %hhu"), &module, &gain)) ||
      (module >= RFM12_MODULE_COUNT))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(module);
  rfm12_setbandwidth(rfm12_modul->rfm12_bandwidth, gain,
                     rfm12_modul->rfm12_drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_setdrssi(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t module;
  uint8_t drssi;
  if ((2 != sscanf_P(cmd, PSTR("%hhu %hhu"), &module, &drssi)) ||
      (module >= RFM12_MODULE_COUNT))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(module);
  rfm12_setbandwidth(rfm12_modul->rfm12_bandwidth, rfm12_modul->rfm12_gain,
                     drssi);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

#ifdef RFM12_IP_SUPPORT
int16_t
parse_cmd_rfm12_reinit(char *cmd, char *output, uint16_t len)
{
  rfm12_net_init();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_setbaud(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint16_t baud;
  if (1 != sscanf_P(cmd, PSTR("%u"), &baud))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(RFM12_MODULE_IP);
  rfm12_setbaud(baud);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_rfm12_setmod(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;

  uint8_t mod;
  if (1 != sscanf_P(cmd, PSTR("%hhu"), &mod))
    return ECMD_ERR_PARSE_ERROR;

  rfm12_prologue(RFM12_MODULE_IP);
  rfm12_setpower(0, mod);
  rfm12_epilogue();

  return ECMD_FINAL_OK;
}
#endif /* RFM12_IP_SUPPORT */

/*
  -- Ethersex META --
  block([[RFM12]])
  ecmd_feature(rfm12_status, "rfm12 status", MODULE, Display internal status of MODULE.)
  ecmd_feature(rfm12_setbandwidth, "rfm12 setbandwidth", MODULE BW, Set receiver bandwidth of MODULE to BW.)
  ecmd_feature(rfm12_setgain, "rfm12 setgain", MODULE GAIN, Set preamplifier gain of MODULE to GAIN.)
  ecmd_feature(rfm12_setdrssi, "rfm12 setdrssi", MODULE DRSSI, Set the drssi of MODULE to DRSSI.)
  ecmd_ifdef(RFM12_IP_SUPPORT)
    ecmd_feature(rfm12_reinit, "rfm12 reinit", , Re-initialize the RFM12 FSK module.)
    ecmd_feature(rfm12_setbaud, "rfm12 setbaud", MODULE BAUD, Set baudrate of MODULE to BAUD.)
    ecmd_feature(rfm12_setmod, "rfm12 setmod", MOD, Set modulation of the RFM12 FSK module to MOD.)
  ecmd_endif()
*/
