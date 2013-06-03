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
#include "hardware/i2c/master/i2c_max7311.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_i2c_max7311_setDDRw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hX"), &adr, &data);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_setDDRw(adr, data);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 setDDRw %u 0x%X"), adr, data));
#else
    return ECMD_FINAL_OK;
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_setOUTw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hX"), &adr, &data);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_setOUTw(adr, data);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 setOUTw %u 0x%X"), adr, data));
#else
    return ECMD_FINAL_OK;
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_getDDRw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getDDRw(adr, &data);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 getDDRw %u 0x%X"), adr, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_getOUTw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getOUTw(adr, &data);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 getOUTw %u 0x%X"), adr, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_getINw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getINw(adr, &data);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 getINw %u 0x%X"), adr, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_set(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t bit;
  uint8_t state;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &adr, &bit, &state);
  if (adr > 0x6F || bit > 15)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_set(adr, bit, state);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 set %u %u %u"), adr, bit,
                  state));
#else
    return ECMD_FINAL_OK;
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t
parse_cmd_i2c_max7311_pulse(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t bit;
  uint16_t time;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hhu %hu"), &adr, &bit, &time);
  if (adr > 0x6F || bit > 15)
    return ECMD_ERR_PARSE_ERROR;
  if (time > 1000)
    time = 1000;
  ret = i2c_max7311_pulse(adr, bit, time);
  if (ret == 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("max7311 pulse %u %u %u"), adr, bit,
                  time));
#else
    return ECMD_FINAL_OK;
#endif
  }
  else
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_max7311_setDDRw, "max7311 setDDRw", ADDR VALUE, Set Direction-Register DDR (VALUE as hex))
  ecmd_feature(i2c_max7311_setOUTw, "max7311 setOUTw", ADDR VALUE, Set Output-Register OUT (VALUE as hex))
  ecmd_feature(i2c_max7311_getDDRw, "max7311 getDDRw", ADDR, Get Direction-Register DDR)
  ecmd_feature(i2c_max7311_getOUTw, "max7311 getOUTw", ADDR, Get Output-Register OUT)
  ecmd_feature(i2c_max7311_getINw, "max7311 getINw", ADDR, Get Input-Register IN)
  ecmd_feature(i2c_max7311_set, "max7311 set", ADDR BIT VALUE, Set Output-BIT to VALUE (bool))
  ecmd_feature(i2c_max7311_pulse, "max7311 pulse", ADDR BIT TIME, Toggle Output-BIT for TIME (in ms))
*/
