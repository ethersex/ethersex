/*
 * Copyright (c) 2018 by Erik Kunze <ethersex@erik-kunze.de>
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

#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "core/util/fixedpoint.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_bmp280.h"
#include "i2c_bmp280_ecmd.h"


static int16_t
i2c_bmp280_map_result(int32_t result, char *output, uint16_t len)
{
  if (result == BMP280_RESULT_OK)
    return ECMD_FINAL_OK;
  else if (result == BMP280_RESULT_NODEV)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor")));
  else if (result == BMP280_RESULT_INVAL)
    return ECMD_ERR_PARSE_ERROR;
  else
    return ECMD_ERR_WRITE_ERROR;
}


int16_t
parse_cmd_i2c_bmp280_gettemp(char *cmd, char *output, uint16_t len)
{
  int16_t temp;

  int8_t result = i2c_bmp280_get_temp(&temp);
  if (result < BMP280_RESULT_OK)
    return i2c_bmp280_map_result(result, output, len);

  return ECMD_FINAL(itoa_fixedpoint(temp, 2, output, len));
}


int16_t
parse_cmd_i2c_bmp280_getpress(char *cmd, char *output, uint16_t len)
{
  uint16_t press;

  int8_t result = i2c_bmp280_get_press(&press);
  if (result < BMP280_RESULT_OK)
    return i2c_bmp280_map_result(result, output, len);

  return ECMD_FINAL(itoa_fixedpoint(press, 1, output, len));
}

#ifdef I2C_BME280_SUPPORT

int16_t
parse_cmd_i2c_bme280_gethumid(char *cmd, char *output, uint16_t len)
{
  uint16_t humid;

  int8_t result = i2c_bme280_get_humid(&humid);
  if (result < BMP280_RESULT_OK)
    return i2c_bmp280_map_result(result, output, len);

  return ECMD_FINAL(itoa_fixedpoint(humid, 1, output, len));
}

#endif

/*
  -- Ethersex META --
  block([[I2C]] (TWI))
  ecmd_feature(i2c_bmp280_gettemp,  "bmp280 temp", , Get temperature in 0.1°C)
  ecmd_feature(i2c_bmp280_getpress, "bmp280 apress", , Get absolute pressure in Pa)
  ecmd_ifdef(I2C_BME280_SUPPORT)
    ecmd_feature(i2c_bme280_gethumid, "bme280 humid", , Get humidity in %RH)
  ecmd_endif()
 */
