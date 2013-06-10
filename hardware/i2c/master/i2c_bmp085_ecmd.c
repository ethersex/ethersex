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
#include "hardware/i2c/master/i2c_bmp085.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_bmp085_temp(char *cmd, char *output, uint16_t len)
{
  int16_t ret = bmp085_get_temp();
  if (ret == -1)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("bmp085 temp %d"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), ret));
#endif
}

int16_t
parse_cmd_i2c_bmp085_apress(char *cmd, char *output, uint16_t len)
{
  int32_t ret = bmp085_get_abs_press();
  if (ret == -1)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("bmp085 apress %ld"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%ld"), ret));
#endif
}

#ifdef I2C_BMP085_BAROCALC_SUPPORT

int16_t
parse_cmd_i2c_bmp085_height(char *cmd, char *output, uint16_t len)
{
  uint32_t pnn;
  int32_t ret;
  if (sscanf_P(cmd, PSTR("%lu"), &pnn) != 1)
    return ECMD_ERR_PARSE_ERROR;

  ret = bmp085_get_abs_press();
  if (ret == -1)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));

  ret = bmp085_get_height_cm(ret, pnn);

#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("bmp085 height %ld"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%ld"), ret));
#endif
}

int16_t
parse_cmd_i2c_bmp085_pressnn(char *cmd, char *output, uint16_t len)
{
  uint32_t height;
  int32_t ret;
  if (sscanf_P(cmd, PSTR("%lu"), &height) != 1)
    return ECMD_ERR_PARSE_ERROR;

  ret = bmp085_get_abs_press();
  if (ret == -1)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));

  ret = bmp085_get_pa_pressure_nn(ret, height);

#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("bmp085 pressnn %ld"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%ld"), ret));
#endif
}

#endif

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_bmp085_temp, "bmp085 temp",, Get temperature in 0.1°C)
  ecmd_feature(i2c_bmp085_apress, "bmp085 apress",, Get absolute pressure in Pa)
  ecmd_ifdef(I2C_BMP085_BAROCALC_SUPPORT)
    ecmd_feature(i2c_bmp085_height, "bmp085 height", PRESSNN, Get height in cm, pressure at N0 needed)
    ecmd_feature(i2c_bmp085_pressnn, "bmp085 pressnn", HEIGHT, Get pressure at N0, height in cm needed)
  ecmd_endif
*/
