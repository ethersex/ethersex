/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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
#include "hardware/i2c/master/i2c_tsl2561.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_tsl2561_getlux(char *cmd, char *output, uint16_t len)
{
  uint8_t devnum, mode;

  if ((sscanf_P(cmd, PSTR("%hhu"), &devnum) != 1) || devnum > 2)
    return ECMD_ERR_PARSE_ERROR;
  int32_t ret = i2c_tsl2561_getlux(devnum);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%ld"), ret));
}

int16_t
parse_cmd_i2c_tsl2561_getraw(char *cmd, char *output, uint16_t len)
{
  uint8_t devnum;
  uint16_t ch0, ch1;

  if ((sscanf_P(cmd, PSTR("%hhu"), &devnum) != 1) || devnum > 2)
    return ECMD_ERR_PARSE_ERROR;
  if (i2c_tsl2561_getluminosity(devnum, &ch0, &ch1))
    return ECMD_ERR_READ_ERROR;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%u %u"), ch0, ch1));
}

int16_t
parse_cmd_i2c_tsl2561_setmode(char *cmd, char *output, uint16_t len)
{
  uint8_t devnum, time, gain, package;

  if ((sscanf_P
       (cmd, PSTR("%hhu %hhu %hhu %hhu"), &devnum, &time, &gain,
        &package) != 4) || devnum > 2)
    return ECMD_ERR_PARSE_ERROR;
  if (i2c_tsl2561_setmode(devnum, time, gain, package))
    return ECMD_ERR_READ_ERROR;
  return ECMD_FINAL_OK;
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_tsl2561_getlux, "tsl2561 lux", DEVNUM, Get LUX value)
  ecmd_feature(i2c_tsl2561_getraw, "tsl2561 raw", DEVNUM, Get RAW channel values)
  ecmd_feature(i2c_tsl2561_setmode, "tsl2561 setmode", DEVNUM TIME GAIN PACKAGE, Set device mode)
*/
