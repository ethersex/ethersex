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
#include "hardware/i2c/master/i2c_bh1750.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_bh1750_lux(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  sscanf_P(cmd, PSTR("%hhu"), &cadr);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t val = i2c_bh1750_read_lux(cadr);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c rwd %d 0x%02X"), cadr, val));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), val));
#endif
}

int16_t
parse_cmd_bh1750_raw(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  sscanf_P(cmd, PSTR("%hhu"), &cadr);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t val = i2c_bh1750_read_lux(cadr);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c rwd %d 0x%02X"), cadr, val));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), val));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(bh1750_lux, "bh1750 lux", CHIPADDR, Get lux)
  ecmd_feature(bh1750_raw, "bh1750 raw", CHIPADDR, Get raw)
*/
