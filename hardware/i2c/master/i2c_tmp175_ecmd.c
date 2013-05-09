/*
 * Copyright (c) 2011 by Patrick Hieber <patrick.hieber@gmx.net>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <string.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_tmp175.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_tmp175(char *cmd, char *output, uint16_t len)
{
  uint8_t chipAdrOffset;
  sscanf_P(cmd, PSTR("%u"), &chipAdrOffset);
  // Return a parse error if out of possible range.
  // There are max. pow(3,3) = 27 different
  // sensors on the i2c bus allowed.
  // @see page 8/20 in the datasheet...
  if (chipAdrOffset > 26)
    return ECMD_ERR_PARSE_ERROR;
  int16_t temp = i2c_tmp175_read_temp(I2C_SLA_TMP175 + chipAdrOffset);
  if (temp == 0xffff)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  uint16_t commaValue = 0;      // holds the digits for decimals
  int8_t wholeNums = (int8_t) (temp >> 8);      // holds the whole numbers
  if (temp & 0x8000)
  {                             // MSB equal 1 -> it's negative
    wholeNums += 1;             // add offset if temp < 0
    commaValue = ((uint8_t) temp == 0) ? 0 : (10000 - (uint8_t) (temp) * 625);
  }
  else
  {
    // leave wholeNums untouched
    commaValue = (uint8_t) temp *625;
  }
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("tmp175 %hhu %3hhd.%04d"),
                               chipAdrOffset, wholeNums, commaValue));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%3hd.%04d"),
                               wholeNums, commaValue));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_tmp175, "tmp175",ADDR, Get temperature)
*/
