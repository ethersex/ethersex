/*
 *
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
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "core/eeprom.h"
#include "core/bit-macros.h"
#include "core/portio/portio.h"
#include "core/portio/named_pin.h"
#include "ecmd.h"

#define NIBBLE_TO_HEX(a) ((a) < 10 ? (a) + '0' : ((a) - 10 + 'A')) 


static uint8_t print_port(char *output, uint8_t len, uint8_t port, uint8_t value) 
{
#ifndef TEENSY_SUPPORT
        return snprintf_P(output, len,
                PSTR("port %d: 0x%02x"),
                port, value);
#else
        memcpy_P(output, PSTR("port P: 0x"), strlen("port P: 0x"));
        /* Convert to number :) */
        output[5] = port + 48;
        output[10] = NIBBLE_TO_HEX((value >> 4) & 0x0F);
        output[11] = NIBBLE_TO_HEX(value & 0x0F);
        return 12;
#endif
}


#ifdef PORTIO_SUPPORT
static uint8_t parse_set_command(char *cmd, uint8_t *port, uint8_t *data, uint8_t *mask) 
{
#ifndef TEENSY_SUPPORT
  return sscanf_P(cmd, PSTR("%x %x %x"),
                  port, data, mask);
#else
  char *p;
  if (! *cmd ) return 0;
  /* skip first space */
  while (*cmd == ' ')
    cmd ++;
  if (! *cmd ) return 0;
  *port = *cmd - '0';
  /* After the second number */
  p = strchr(cmd, ' ');
  if (! p) return 1;
  /* skip spaces */
  while (*p == ' ')
    p++;
  cmd = p;
  *data = strtol(cmd, NULL, 16);
  p = strchr(cmd, ' ');
  if (! p) return 2;
  /* skip spaces */
  while (*p == ' ')
    p++;
  cmd = p;
  *mask = strtol(cmd, NULL, 16);
  return 3;
#endif
}


int16_t parse_cmd_io_set_ddr(char *cmd, char *output, uint16_t len)
{
    (void) output;
    (void) len;

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_set_ddr with rest: \"%s\"\n", cmd);
#endif

    uint8_t port, data, mask;

    uint8_t ret = parse_set_command(cmd, &port, &data, &mask);
    /* use default mask, if no mask has been given */
    if (ret == 2) {
        mask = 0xff;
        ret = 3;
    }

    if (ret == 3 && port < IO_PORTS && vport[port].write_ddr 
        && vport[port].read_ddr) {
        vport[port].write_ddr(port, (vport[port].read_ddr(port) & ~mask)
                              | LO8(data & mask));

        return 0;
    } else
        return -1;

}

int16_t parse_cmd_io_get_mask(char *cmd, char *output, uint16_t len)
{

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_ddr with rest: \"%s\"\n", cmd);
#endif

    uint8_t port;
#ifndef TEENSY_SUPPORT
    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);
    if (ret == 1 && port < IO_PORTS && vport[port].read_ddr) 
#else
    port = *(cmd + 1) - '0';
    if (port < IO_PORTS)
#endif
      return print_port(output, len, port, vport[port].mask);
    else
      return -1;

}

int16_t parse_cmd_io_get_ddr(char *cmd, char *output, uint16_t len)
{

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_ddr with rest: \"%s\"\n", cmd);
#endif

    uint8_t port;
#ifndef TEENSY_SUPPORT
    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);
    if (ret == 1 && port < IO_PORTS && vport[port].read_ddr) 
#else
    port = *(cmd + 1) - '0';
    if (port < IO_PORTS && vport[port].read_ddr)
#endif
      return print_port(output, len, port, vport[port].read_ddr(port));
    else
      return -1;

}

int16_t parse_cmd_io_set_port(char *cmd, char *output, uint16_t len)
{
    (void) output;
    (void) len;

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_set_port with rest: \"%s\"\n", cmd);
#endif

    uint8_t port, data, mask;
    uint8_t ret = parse_set_command(cmd, &port, &data, &mask);
    /* use default mask, if no mask has been given */
    if (ret == 2) {
        mask = 0xff;
        ret = 3;
    }

    if (ret == 3 && port < IO_PORTS && vport[port].write_port 
        && vport[port].read_port) {
        vport[port].write_port(port, (vport[port].read_port(port) & ~mask)
                               | LO8(data & mask));
        return 0;
    } else
        return -1;

}

int16_t parse_cmd_io_get_port(char *cmd, char *output, uint16_t len)
{

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_port with rest: \"%s\"\n", cmd);
#endif

    uint8_t port;

#ifndef TEENSY_SUPPORT
    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);
    if (ret == 1 && port < IO_PORTS && vport[port].read_port) 
#else
    port = *(cmd + 1) - '0';
    if (port < IO_PORTS && vport[port].read_port)
#endif
      return print_port(output, len, port, vport[port].read_port(port));
    else
        return -1;

}

int16_t parse_cmd_io_get_pin(char *cmd, char *output, uint16_t len)
{

#ifdef DEBUG_ECMD_PORTIO
    debug_printf("called parse_cmd_io_get_pin with rest: \"%s\"\n", cmd);
#endif

    uint8_t port;

#ifndef TEENSY_SUPPORT
    int ret = sscanf_P(cmd,
            PSTR("%x"),
            &port);
    if (ret == 1 && port < IO_PORTS && vport[port].read_pin) 
#else
    port = *(cmd + 1) - '0';
    if (port < IO_PORTS && vport[port].read_pin)
#endif
      return print_port(output, len, port, vport[port].read_pin(port));
    else
        return -1;

}
#endif /* PORTIO_SUPPORT */

#ifdef PORTIO_SIMPLE_SUPPORT

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
    default: return -1;
  }
  /* skip non spaces */
  while (*cmd != ' ')
    cmd ++;
  /* skip spaces */
  while (*cmd == ' ')
    cmd ++;
  /* skip first char of ddr,port,pin*/
  cmd ++;
  /* test of p'i'n, d'd'r p'o'rt or m'a'sk case insensitiv */
  switch (*cmd & 0xDF)
  {
    case 'I' : iotypeoffset = 0; break;
    case 'D' : iotypeoffset = 1; break;
    case 'O' : iotypeoffset = 2; break;
#ifndef TEENSY_SUPPORT
    case 'A' : iotypeoffset = 3; cmd += 3; break;
#endif
    default: return -1;
  }
  cmd ++;
  /* skip the rest of registertyp and spaces*/
  while (*cmd == ' ' || (*cmd & 0xDF) >= 'N')
    cmd ++;
  /* get the port number */
  cmd = parse_hex(cmd, &value);
  if (cmd == 0)
    return -1;
  /* translate it to the portaddress */
  switch (value)
  {
#ifdef PINA
    case 0: ioptr = &PINA; sysmask = PORTIO_MASK_A; break;
    case 1: ioptr = &PINB; sysmask = PORTIO_MASK_B; break;
    case 2: ioptr = &PINC; sysmask = PORTIO_MASK_C; break;
    case 3: ioptr = &PIND; sysmask = PORTIO_MASK_D; break;
#else
    case 0: ioptr = &PINB; sysmask = PORTIO_MASK_B; break;
    case 1: ioptr = &PINC; sysmask = PORTIO_MASK_C; break;
    case 2: ioptr = &PIND; sysmask = PORTIO_MASK_D; break;
#endif
    default: return -1;
  }
  ioptr += iotypeoffset;
#ifndef TEENSY_SUPPORT
  if (iotypeoffset == 3) 
    return print_port(output, len, value, ~sysmask);
#endif
  if(getorset)
    /* wenn get request return the port value */
    return print_port(output, len, value, *ioptr);
  /* get register write value */
  cmd = parse_hex(cmd, &value);
  if (cmd == 0)
    return -1;
  /* if a mask value present get it */
  parse_hex(cmd, &mask);
  *ioptr = (*ioptr & ~(mask & sysmask)) | (value & mask & sysmask);
  return 0;
}
#endif /* PORTIO_SIMPLE_SUPPORT */


#if defined(NAMED_PIN_SUPPORT) && defined(PORTIO_SUPPORT)
int16_t parse_cmd_pin_get(char *cmd, char *output, uint16_t len)
{
  uint16_t port, pin;

  uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &port, &pin);
  /* Fallback to named pins */
  if ( ret != 2 && *cmd) {
    uint8_t pincfg = named_pin_by_name(cmd + 1);
    if (pincfg != 255) {
        port = pgm_read_byte(&portio_pincfg[pincfg].port);
        pin = pgm_read_byte(&portio_pincfg[pincfg].pin);
        ret = 2;
    }
  }
  if (ret == 2 && port < IO_PORTS && pin < 8) {
    uint8_t pincfg = named_pin_by_pin(port, pin);
    uint8_t active_high = 1;
    if (pincfg != 255)  
      active_high = pgm_read_byte(&portio_pincfg[pincfg].active_high);
    return snprintf_P(output, len, 
                      XOR_LOG(vport[port].read_pin(port) & _BV(pin), !(active_high))
                      ? PSTR("on") : PSTR("off"));
  } else
    return -1;
}
/* */

int16_t parse_cmd_pin_set(char *cmd, char *output, uint16_t len)
{
  uint16_t port, pin, on;

  /* Parse String */
  uint8_t ret = sscanf_P(cmd, PSTR("%u %u %u"), &port, &pin, &on);
  /* Fallback to named pins */
  if ( ret != 3 && *cmd) {
    char *ptr = strchr(cmd + 1, ' ');
    if (ptr) {
      *ptr = 0;
      uint8_t pincfg = named_pin_by_name(cmd + 1);
      if (pincfg != 255) {
        port = pgm_read_byte(&portio_pincfg[pincfg].port);
        pin = pgm_read_byte(&portio_pincfg[pincfg].pin);
        if (ptr[1]) {
          ptr++;
          if(sscanf_P(ptr, PSTR("%u"), &on) == 1)
            ret = 3;
          else {
            if (strcmp_P(ptr, PSTR("on")) == 0) {
              on = 1;
              ret = 3;
            }
            else if (strcmp_P(ptr, PSTR("off")) == 0) {
              on = 0;
              ret = 3;
            }
          }
        }
      }
    }
  }

  if (ret == 3 && port < IO_PORTS && pin < 8) {
    /* Set only if it is output */
    if (vport[port].read_ddr(port) & _BV(pin)) {
      uint8_t pincfg = named_pin_by_pin(port, pin);
      uint8_t active_high = 1;
      if (pincfg != 255)  
        active_high = pgm_read_byte(&portio_pincfg[pincfg].active_high);

      if (XOR_LOG(on, !active_high)) 
        vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));
      else
        vport[port].write_port(port, vport[port].read_port(port) & ~_BV(pin));

      return snprintf_P(output, len, on ? PSTR("on") : PSTR("off"));
    } else 
      return snprintf_P(output, len, PSTR("error: pin is input"));

  } else
    return -1;
}
/* */

int16_t parse_cmd_pin_toggle(char *cmd, char *output, uint16_t len)
{
  uint16_t port, pin;

  /* Parse String */
  uint8_t ret = sscanf_P(cmd, PSTR("%u %u"), &port, &pin);
  /* Fallback to named pins */
  if ( ret != 2 && *cmd) {
    uint8_t pincfg = named_pin_by_name(cmd + 1);
    if (pincfg != 255) {
        port = pgm_read_byte(&portio_pincfg[pincfg].port);
        pin = pgm_read_byte(&portio_pincfg[pincfg].pin);
        ret = 2;
    }
  }
  if (ret == 2 && port < IO_PORTS && pin < 8) {
    /* Toggle only if it is output */
    if (vport[port].read_ddr(port) & _BV(pin)) {
      uint8_t on = vport[port].read_port(port) & _BV(pin);

      uint8_t pincfg = named_pin_by_pin(port, pin);
      uint8_t active_high = 1;
      if (pincfg != 255)  
        active_high = pgm_read_byte(&portio_pincfg[pincfg].active_high);

      if (on) 
        vport[port].write_port(port, vport[port].read_port(port) & ~_BV(pin));
      else
        vport[port].write_port(port, vport[port].read_port(port) | _BV(pin));

      return snprintf_P(output, len, XOR_LOG(!on, !active_high)
                        ? PSTR("on") : PSTR("off"));
    } else 
      return snprintf_P(output, len, PSTR("error: pin is input"));

  } else
    return -1;
}
/* */
#endif /* NAMED_PIN_SUPPORT && PORTIO_SUPPORT */

