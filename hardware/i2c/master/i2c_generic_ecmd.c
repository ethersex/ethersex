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
#include "hardware/i2c/master/i2c_generic.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_read_byte(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr < 7 || adr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint8_t val = i2c_read_byte(adr);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("i2c rbb %d 0x%02X"), adr, val));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), val));
#endif
}

int16_t
parse_cmd_i2c_read_byte_data(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  uint8_t dadr;
  sscanf_P(cmd, PSTR("%hhu %hhu"), &cadr, &dadr);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint8_t val = i2c_read_byte_data(cadr, dadr);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c rbd %d %d 0x%02X"), cadr, dadr, val));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), val));
#endif
}

int16_t
parse_cmd_i2c_read_word_data(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  uint8_t dadr;
  sscanf_P(cmd, PSTR("%hhu %hhu"), &cadr, &dadr);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t val = i2c_read_word_data(cadr, dadr);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c rwd %d %d 0x%02X"), cadr, dadr, val));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), val));
#endif
}

int16_t
parse_cmd_i2c_write_byte(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t data;
  sscanf_P(cmd, PSTR("%hhu %hhx"), &adr, &data);
  if (adr < 7 || adr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t ret = i2c_write_byte(adr, data);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P(output, len, PSTR("i2c wbb %d 0x%02X"), adr, ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

int16_t
parse_cmd_i2c_write_byte_data(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  uint8_t dadr;
  uint8_t data;
  sscanf_P(cmd, PSTR("%hhu %hhu %hhx"), &cadr, &dadr, &data);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t ret = i2c_write_byte_data(cadr, dadr, data);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c wbd %d %d 0x%02X"), cadr, dadr, ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

int16_t
parse_cmd_i2c_write_word_data(char *cmd, char *output, uint16_t len)
{
  uint8_t cadr;
  uint8_t dadr;
  uint16_t data;
  sscanf_P(cmd, PSTR("%hhu %hhu %x"), &cadr, &dadr, &data);
  if (cadr < 7 || cadr > 127)
    return ECMD_ERR_PARSE_ERROR;
  uint16_t ret = i2c_write_word_data(cadr, dadr, data);
#ifdef ECMD_MIRROR_REQUEST
  return
    ECMD_FINAL(snprintf_P
               (output, len, PSTR("i2c wwd %d %d 0x%02X"), cadr, dadr, ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%02X"), ret));
#endif
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_read_byte, "i2c rbb",ADDR,read byte from I2C chip)
  ecmd_feature(i2c_read_byte_data, "i2c rbd",CHIPADDR REGADDR,read byte from register address at I2C chip)
  ecmd_feature(i2c_read_word_data, "i2c rwd",CHIPADDR REGADDR,read word from register address at I2C chip)
  ecmd_feature(i2c_write_byte, "i2c wbb",ADDR HEXVALUE,write byte to I2C chip)
  ecmd_feature(i2c_write_byte_data, "i2c wbd",CHIPADDR REGADDR HEXVALUE,write byte to register address on I2C chip)
  ecmd_feature(i2c_write_word_data, "i2c wwd",CHIPADDR REGADDR HEXVALUE,write word to register address on I2C chip)
*/
