/*
 *
 * Copyright (c) 2012 by Daniel Walter <fordprfkt@googlemail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pca9555.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_pca9555_out(char *cmd, char *output, uint16_t len)
{
  uint16_t data;
  sscanf_P(cmd, PSTR("%u"), &data);

  uint16_t ret = i2c_pca9555_writeOutPort(0, data);

#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("i2c pca9555 out 0x%02X"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

int16_t
parse_cmd_i2c_pca9555_in(char *cmd, char *output, uint16_t len)
{
  uint16_t data = 0;

  uint16_t ret = i2c_pca9555_readInPort(0, &data);

#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("i2c pca9555 in 0x%02X"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

int16_t
parse_cmd_i2c_pca9555_mode(char *cmd, char *output, uint16_t len)
{
  uint16_t data;
  sscanf_P(cmd, PSTR("%u"), &data);

  uint16_t ret = i2c_pca9555_setDDR(0, data);

#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("i2c pca9555 mode 0x%02X"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

/*
  -- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_pca9555_out, "i2c pca9555 out",VALUE,write word to register address on I2C chip)
  ecmd_feature(i2c_pca9555_in, "i2c pca9555 in",,read word from register address on I2C chip)
  ecmd_feature(i2c_pca9555_mode, "i2c pca9555 mode",VALUE,select input or output mode for pins on I2C chip)
*/
