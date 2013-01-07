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
#include "hardware/i2c/master/i2c_pcf8574x.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_pcf8574x_read(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t chip;
  sscanf_P(cmd, PSTR("%hhu %hhu"), &adr, &chip);

#ifdef ECMD_MIRROR_REQUEST
  uint8_t oadr = adr;
#endif

  if (chip == 0)
  {
    adr += I2C_SLA_PCF8574;
  }
  else
  {
    adr += I2C_SLA_PCF8574A;
  }
#ifdef DEBUG_I2C
  debug_printf("I2C PCF8574X IC address 0x%X\n", adr);
#endif
#ifdef ECMD_MIRROR_REQUEST
  uint8_t rc = i2c_pcf8574x_read(adr);
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("pcf8574x read %u %u 0x%X"), oadr, chip,
                rc));
#else
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), i2c_pcf8574x_read(adr)));
#endif
}

int16_t
parse_cmd_i2c_pcf8574x_set(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t chip;
  uint8_t value;
  sscanf_P(cmd, PSTR("%hhu %hhu %hhx"), &adr, &chip, &value);

#ifdef ECMD_MIRROR_REQUEST
  uint8_t oadr = adr;
#endif

  if (chip == 0)
  {
    adr += I2C_SLA_PCF8574;
  }
  else
  {
    adr += I2C_SLA_PCF8574A;
  }
#ifdef DEBUG_I2C
  debug_printf("I2C PCF8574X IC address 0x%X, value:%X\n", adr, value);
#endif
  i2c_pcf8574x_set(adr, value);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("pcf8574x set %u %u 0x%X"), oadr, chip,
                value));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), value));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_pcf8574x_read, "pcf8574x read",ADDR CHIP, Get bits)
  ecmd_feature(i2c_pcf8574x_set, "pcf8574x set",ADDR CHIP HEXVALUE, Set bits)
*/
