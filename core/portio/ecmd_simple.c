/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"
#include "core/portio/named_pin.h"

#include "protocols/ecmd/ecmd-base.h"
#include "ecmd_base.c"


static char* parse_hex(char *text, uint8_t *value)
{
  if (! *text ) return 0;
  uint8_t nibble;
  /* skip spaces */
  while (*text == ' ')
    text ++;
  if (! *text ) return 0;
  *value = 0;
  while ((*text >= '0' && *text <= '9') || ((*text & 0xDF) >= 'A' && (*text & 0xDF) <= 'F'))
  {
    *value <<= 4;
    nibble = *text - '0';
    if (nibble > 9)
      nibble -= 7;
    *value |= nibble;
    text++;
  }
  return text;
}


int16_t parse_cmd_io(char *cmd, char *output, uint16_t len)
{
  (void) output;
  (void) len;
  
#ifdef DEBUG_ECMD_PORTIO
  debug_printf("called parse_cmd_io_set with rest: \"%s\"\n", cmd);
#endif
  
  volatile uint8_t *ioptr;
  uint8_t getorset;
  uint8_t iotypeoffset;
  uint8_t value;
  uint8_t mask = 0xFF;
  uint8_t sysmask = 0;
  
  /* skip spaces */
  while (*cmd == ' ')
    cmd ++;
  /* test of 'g'et or 's'et */
  switch (*cmd)
  {
    case 'g': getorset = 1; break;
    case 's': getorset = 0; break;
    default: return ECMD_ERR_PARSE_ERROR;
  }
  /* skip non spaces */
  while (*cmd != ' ')
    cmd ++;
  /* skip spaces */
  while (*cmd == ' ')
    cmd ++;
  /* skip first char of ddr,port,pin*/
  cmd ++;
  /* test of p'i'n, d'd'r, p'o'rt or m'a'sk case insensitiv */
  switch (*cmd & 0xDF)
  {
    case 'I' : iotypeoffset = 0; break;
    case 'D' : iotypeoffset = 1; break;
    case 'O' : iotypeoffset = 2; break;
#ifndef TEENSY_SUPPORT
    case 'A' : iotypeoffset = 3; cmd += 3; break;
#endif
    default: return ECMD_ERR_PARSE_ERROR;
  }
  cmd ++;
  /* skip the rest of registertyp and spaces*/
  while (*cmd == ' ' || (*cmd & 0xDF) >= 'N')
    cmd ++;
  /* get the port number */
  cmd = parse_hex(cmd, &value);
  if (cmd == 0)
    return ECMD_ERR_PARSE_ERROR;
  /* translate it to the portaddress */
  
#ifdef PINA
  switch (value)
#else
  switch (value + 1)
#endif
  {
#ifdef PINA
    case  0: ioptr = &PINA; sysmask = PORTIO_MASK_A; break;
#endif
#ifdef PINB
    case  1: ioptr = &PINB; sysmask = PORTIO_MASK_B; break;
#endif
#ifdef PINC
    case  2: ioptr = &PINC; sysmask = PORTIO_MASK_C; break;
#endif
#ifdef PIND
    case  3: ioptr = &PIND; sysmask = PORTIO_MASK_D; break;
#endif
#ifdef PINE
    case  4: ioptr = &PINE; sysmask = PORTIO_MASK_E; break;
#endif
#ifdef PINF
    case  5: ioptr = &PINF; sysmask = PORTIO_MASK_F; break;
#endif
#ifdef PING
    case  6: ioptr = &PING; sysmask = PORTIO_MASK_G; break;
#endif
#ifdef PINH
    case  7: ioptr = &PINH; sysmask = PORTIO_MASK_H; break;
#endif
#ifdef PINJ
    case  8: ioptr = &PINJ; sysmask = PORTIO_MASK_J; break;
#endif
#ifdef PINK
    case  9: ioptr = &PINK; sysmask = PORTIO_MASK_K; break;
#endif
#ifdef PINL
    case 10: ioptr = &PINL; sysmask = PORTIO_MASK_L; break;
#endif
    default: return ECMD_ERR_PARSE_ERROR;
  }
  ioptr += iotypeoffset;
#ifndef TEENSY_SUPPORT
  if (iotypeoffset == 3) 
    return ECMD_FINAL(print_port(output, len, value, ~sysmask));
#endif
  if(getorset)
    /* when get request return the port value */
    return ECMD_FINAL(print_port(output, len, value, *ioptr));
  /* get register write value */
  cmd = parse_hex(cmd, &value);
  if (cmd == 0)
    return ECMD_ERR_PARSE_ERROR;
  /* if a mask value present get it */
  parse_hex(cmd, &mask);
  *ioptr = (*ioptr & ~(mask & sysmask)) | (value & mask & sysmask);
  return ECMD_FINAL_OK;
}

/*
  -- Ethersex META --
  ecmd_feature(io, "io")
*/
