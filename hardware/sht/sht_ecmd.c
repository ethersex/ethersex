/*
* ECMD-commands to handle reading SHT humidity & temp sensors
*
* Copyright (c) 2010 Gerd v. Egidy <gerd@egidy.de>
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/util/fixedpoint.h"
#include "protocols/ecmd/ecmd-base.h"

#include "sht.h"
#include "sht_ecmd.h"

// output error messages to ECMDs output buffer
static int16_t
output_sht_error(int8_t returncode, char *output)
{
  switch (returncode)
  {
    case SHT_ERR_CRC:
#define SHT_ERR_CRC_MSG "crc error"
      strcpy_P(output, PSTR(SHT_ERR_CRC_MSG));
      return ECMD_FINAL(sizeof(SHT_ERR_CRC_MSG));
    case SHT_ERR_TIMEOUT:
#define SHT_ERR_TIMEOUT_MSG "timeout"
      strcpy_P(output, PSTR(SHT_ERR_TIMEOUT_MSG));
      return ECMD_FINAL(sizeof(SHT_ERR_TIMEOUT_MSG));
    case SHT_ERR_PROTOCOL:
    default:
#define SHT_ERR_PROTOCOL_MSG "no sht"
      strcpy_P(output, PSTR(SHT_ERR_PROTOCOL_MSG));
      return ECMD_FINAL(sizeof(SHT_ERR_PROTOCOL_MSG));
  }
}

int16_t
parse_cmd_sht_raw(char *cmd, char *output, uint16_t len)
{
  uint16_t raw_temp = 0;
  uint16_t raw_humid = 0;
  int8_t ret;

  // the longest message is 10 bytes + \0
  if (len < 11)
    return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

  ret = sht_get(&raw_temp, &raw_humid);

  if (ret == SHT_OK)
  {
    itoa(raw_temp, output, 16);
    ret = strlen(output);
    output[ret++] = '\r';
    output[ret++] = '\n';
    itoa(raw_humid, output + ret, 16);
    return ECMD_FINAL(strlen(output));
  }
  else
    return ECMD_FINAL(output_sht_error(ret, output));
}

int16_t
parse_cmd_sht_temp(char *cmd, char *output, uint16_t len)
{
  uint16_t raw_temp = 0;
  int8_t ret;

  // the longest error message is 9 bytes + \0, all possible temp/hex values fit
  if (len < 10)
    return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

  ret = sht_get(&raw_temp, NULL);

  if (ret == SHT_OK)
    return
      ECMD_FINAL(itoa_fixedpoint(sht_convert_temp(&raw_temp), 2, output, len));
  else
    return ECMD_FINAL(output_sht_error(ret, output));
}

int16_t
parse_cmd_sht_humid(char *cmd, char *output, uint16_t len)
{
  uint16_t raw_temp = 0;
  uint16_t raw_humid = 0;
  int8_t ret;

  // the longest error message is 9 bytes + \0, all possible temp/hex values fit
  if (len < 10)
    return ECMD_FINAL(ECMD_ERR_PARSE_ERROR);

  ret = sht_get(&raw_temp, &raw_humid);

  if (ret == SHT_OK)
    return
      ECMD_FINAL(itoa_fixedpoint
                 (sht_convert_humid(&raw_temp, &raw_humid), 1, output, len));
  else
    return ECMD_FINAL(output_sht_error(ret, output));
}


/*
  -- Ethersex META --
  block([[SHT]])
  ecmd_ifdef(SHT_RAW_SUPPORT)
    ecmd_feature(sht_raw, "sht raw",, Return raw hex temp (first line) and humidity value (second line) of SHT sensor)
  ecmd_endif()
  ecmd_ifdef(SHT_TEMP_SUPPORT)
    ecmd_feature(sht_temp, "sht temp",, Return temperature of SHT sensor)
  ecmd_endif()
  ecmd_ifdef(SHT_HUMID_SUPPORT)
    ecmd_feature(sht_humid, "sht humid",, Return humidity of SHT sensor)
  ecmd_endif()
*/
