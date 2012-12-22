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
#include "hardware/i2c/master/i2c_lm75.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_lm75(char *cmd, char *output, uint16_t len)
{
  while (*cmd == ' ')
    cmd++;
  if (*cmd < '0' || *cmd > '7')
    return ECMD_ERR_PARSE_ERROR;
  int16_t temp = i2c_lm75_read_temp(I2C_SLA_LM75 + (cmd[0] - '0'));
  if (temp == 0xffff)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("lm75 %d %d.%d"), (cmd[0] - '0'), temp / 10,
                temp % 10));
#else
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("%d.%d"), temp / 10, temp % 10));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_lm75, "lm75",ADDR, Get temperature)
*/
