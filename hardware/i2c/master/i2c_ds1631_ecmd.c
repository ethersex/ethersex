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
#include "hardware/i2c/master/i2c_ds1631.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_ds1631_set_power_state(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t state;
  sscanf_P(cmd, PSTR("%hhu %hhu"), &adr, &state);
  if (adr > 7)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t temp = i2c_ds1631_start_stop(I2C_SLA_DS1631 + adr, state);
  if (temp == 0xffff)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("ds1631 convert %d %d"), adr, state));
#else
  return ECMD_FINAL_OK;
#endif
}

int16_t
parse_cmd_i2c_ds1631_read_temperature(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  int16_t temp;
  int16_t stemp;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 7)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t ret =
    i2c_ds1631_read_temperature(I2C_SLA_DS1631 + adr, &temp, &stemp);
  if (ret == 0xffff)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("ds1631 temp %d %d.%d"), adr, temp, stemp));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d.%d"), temp, stemp));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_ds1631_read_temperature, "ds1631 temp",ADDR, Read last converted temperature)
  ecmd_feature(i2c_ds1631_set_power_state, "ds1631 convert",ADDR VALUE, Initiate temperature conversions (0: stop, 1: convert))
*/
