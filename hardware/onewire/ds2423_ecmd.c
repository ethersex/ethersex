/*
 * Support for the counters of DS2423
 * ECMD interface
 * Copyright (C) 2009 Meinhard Schneider <meini@meini.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

#include "ds2423.h"
#include "onewire.h"

/* from ecmd.c: */
int8_t parse_ow_rom(char *cmd, ow_rom_code_t * rom);

/* parse the command line and check for a rom code */
static int8_t
ow_ecmd_parse_ds2423_rom_arg(char **cmd, ow_rom_code_t *ptr_rom)
{
  int8_t ret;

  while (**cmd == ' ')
    (*cmd)++;

  if (strlen(*cmd) >= 16)
  {
    /* called with rom code */
    ret = parse_ow_rom(*cmd, ptr_rom);

    /* check for parse error */
    if (ret < 0)
      return ECMD_ERR_PARSE_ERROR;

    /* move pointer behind rom code */
    *cmd += 16;
  }
  else
  {
    /* no rom code, use skip command */
    ptr_rom->raw = 0;
  }

  while (**cmd == ' ')
    (*cmd)++;

  return 0;
}

/* ECMD handler: "1w ds2423 counter" - get value of given counter */
int16_t
parse_cmd_onewire_ds2423_get_counter(char *cmd, char *output, uint16_t len)
{
  ow_rom_code_t rom;
  int8_t ret;
  uint8_t ecmd_return_len = 0;
  uint32_t result;
  int8_t counter;

  ret = ow_ecmd_parse_ds2423_rom_arg(&cmd, &rom);

  if (ret != 0)
    return ECMD_ERR_READ_ERROR;

  /* output ROM code */
  for (uint8_t i = 0; i < 8; i++)
  {
    ret = snprintf_P(output, len, PSTR("%hhu "), rom.bytewise[i]);
    ecmd_return_len += ret;
    output += ret;
    len -= ret;
  }

  if (rom.raw == 0)
    ret = ow_skip_rom();
  else
  {
    /* check for known family code */
    if (!ow_ds2423_sensor(&rom))
      return ECMD_ERR_PARSE_ERROR;

    ret = ow_match_rom(&rom);
  }

  if (ret < 0)
    return ECMD_ERR_READ_ERROR;

  if (*cmd == '\0')
    /* get all counters */
    counter = -1;
  else if (sscanf_P(cmd, PSTR("%hhd"), &counter) != 1)
    return ECMD_ERR_PARSE_ERROR;

  if (counter >= 0)
  {
    uint8_t sreg = SREG;
    cli();
    ret = ow_ds2423_get_counter(&rom, counter, &result);
    SREG = sreg;
    if (ret > 0)
      ecmd_return_len += snprintf_P(output, len,
                                    PSTR("%c: %lu"), (unsigned char) ret + 65,
                                    result);
  }
  else if (counter > -15)
  {
    /* read range of counters */
    for (uint8_t i = -counter - 1; i < OW_DS2423_PAGE_COUNT; i++)
    {
      uint8_t sreg = SREG;
      cli();
      ret = ow_ds2423_get_counter(&rom, i, &result);
      SREG = sreg;
      if (ret > 0)
      {
        ret = snprintf_P(output, len, PSTR("%c: %lu "),
                         (unsigned char) ret + 65, result);
        ecmd_return_len += ret;
        output += ret;
        len -= ret;
      }
      else
        break;
    }
  }
  else
    return ECMD_ERR_READ_ERROR;

  return ECMD_FINAL(ecmd_return_len);
}

/*
 * -- Ethersex META --
 * block([[Dallas_1-wire_Bus]])
 * ecmd_feature(onewire_ds2423_get_counter, "1w ds2423 counter", [DEVICE], `get value of given counter')
 */
