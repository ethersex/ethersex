/*
 * Copyright (c) 2010 by Christian Dietrich <stettberger@dokucode.de>
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

#include <stdint.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "core/global.h"
#include "core/eeprom.h"
#include "protocols/ecmd/ecmd-base.h"
#include "core/util/string_parsing.h"

#define eeprom_start (sizeof(struct eeprom_config_t))

int16_t
parse_cmd_eer(char *cmd, char *output, uint16_t len)
{
  uint16_t addr_offset, length;

  uint8_t p = next_uint16(cmd, &addr_offset);
  if (p == 0)
    return ECMD_ERR_PARSE_ERROR;

  p = next_uint16(cmd + p, &length);
  if (p == 0)
    length = 1;

  if (length * 2 >= len)
    length = len / 2;

  if (0xffff - eeprom_start - length < addr_offset)
    return ECMD_ERR_PARSE_ERROR;

  uint16_t ptr = eeprom_start + addr_offset;
  for (uint16_t i = 0; i < length; i++)
    sprintf_P(output + (i << 1), PSTR("%02x"),
              eeprom_read_byte((uint8_t *) (ptr++)));

  return ECMD_FINAL(length * 2);
}



int16_t
parse_cmd_eew(char *cmd, char *output, uint16_t len)
{
  uint16_t addr_offset;

  uint8_t p = next_uint16(cmd, &addr_offset);
  if (p == 0)
    return ECMD_ERR_PARSE_ERROR;

  cmd += p;

  uint16_t ptr = eeprom_start + addr_offset;
  char *cmd_end = cmd + strlen(cmd);

  uint8_t i = 0;
  while (cmd < cmd_end)
  {
    if (ptr < eeprom_start)
      return ECMD_ERR_PARSE_ERROR;

    /* Read the next hex byte */
    uint8_t value;
    if (!(p = next_hexbyte(cmd, &value)))
      return sprintf(output, "%d hexbyte '%x'", i, value);
    cmd += p;

    eeprom_write_byte((uint8_t *) (ptr++), value);
    i++;
  }

  return ECMD_FINAL_OK;
}


/*
  -- Ethersex META --
  block(Reading and Writing EEPROM Space on Device)
  ecmd_feature(eer, "eer",eer <ADDR> <LENGTH>, Read n bytes from address after the config in eeprom.)
  ecmd_feature(eew, "eew",eew <ADDR> <HEXBYTES>, Write Hexbytes at address after the config in epprom.)
*/
