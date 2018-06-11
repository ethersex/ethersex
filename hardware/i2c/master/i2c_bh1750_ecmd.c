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
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_bh1750.h"
#include "i2c_bh1750_ecmd.h"


static int16_t
i2c_bh1750_map_result(int32_t result, char *output, uint16_t len)
{
  if (result == BH1750_RESULT_OK)
    return ECMD_FINAL_OK;
  else if (result == BH1750_RESULT_NODEV)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor")));
  else if (result == BH1750_RESULT_INVAL)
    return ECMD_ERR_PARSE_ERROR;
  else
    return ECMD_ERR_WRITE_ERROR;
}


int16_t
parse_cmd_i2c_bh1750_setmode(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;

  i2c_bh1750_resolution resolution, auto_power_down;

  if ((sscanf_P(cmd, PSTR("%hhu %hhu"), &resolution, &auto_power_down) != 2)
#ifdef I2C_BH1750_AUTO_RESOLUTION
      || (resolution > RESOLUTION_HIGH && resolution != RESOLUTION_AUTO_HIGH)
#else
      || (resolution > RESOLUTION_HIGH)
#endif
      || (auto_power_down > 1))
    return ECMD_ERR_PARSE_ERROR;

  int16_t result = i2c_bh1750_set_operating_mode(resolution, auto_power_down);
  return i2c_bh1750_map_result(result, output, len);
}


int16_t
parse_cmd_i2c_bh1750_getlux(char *cmd, char *output, uint16_t len)
{
  int32_t lux = i2c_bh1750_get_lux();
  if (lux < BH1750_RESULT_OK)
    return i2c_bh1750_map_result(lux, output, len);

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%li"), lux));
}


/*
  -- Ethersex META --
  block([[I2C]] (TWI))
  ecmd_feature(i2c_bh1750_setmode, "bh1750 mode", RESOLUTION AUTOPOWERDOWN, Set sensor mode)
  ecmd_feature(i2c_bh1750_getlux, "bh1750 lux", , Get lux value)
 */
