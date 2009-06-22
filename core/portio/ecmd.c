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
#include "core/bit-macros.h"
#include "core/portio/portio.h"
#include "core/portio/named_pin.h"

#include "protocols/ecmd/ecmd-base.h"
#include "ecmd_base.c"


static uint8_t parse_set_command(char *cmd, uint8_t *port, uint8_t *data, uint8_t *mask) 
{
#ifndef TEENSY_SUPPORT
  return ECMD_FINAL(sscanf_P(cmd, PSTR("%x %x %x"), port, data, mask));
#else
  char *p;
  if (! *cmd ) return ECMD_FINAL_OK;
  /* skip first space */
  while (*cmd == ' ')
    cmd ++;
  if (! *cmd ) return ECMD_FINAL_OK;
  *port = *cmd - '0';
  /* After the second number */
  p = strchr(cmd, ' ');
  if (! p) return ECMD_FINAL(1);
  /* skip spaces */
  while (*p == ' ')
    p++;
  cmd = p;
  *data = strtol(cmd, NULL, 16);
  p = strchr(cmd, ' ');
  if (! p) return ECMD_FINAL(2);
  /* skip spaces */
  while (*p == ' ')
    p++;
  cmd = p;
  *mask = strtol(cmd, NULL, 16);
  return ECMD_FINAL(3);
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

        return ECMD_FINAL_OK;
    } else
        return ECMD_ERR_PARSE_ERROR;

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
      return ECMD_FINAL(print_port(output, len, port, vport[port].mask));
    else
      return ECMD_ERR_PARSE_ERROR;

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
      return ECMD_FINAL(print_port(output, len, port, vport[port].read_ddr(port)));
    else
      return ECMD_ERR_PARSE_ERROR;

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
        return ECMD_FINAL_OK;
    } else
        return ECMD_ERR_PARSE_ERROR;

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
      return ECMD_FINAL(print_port(output, len, port, vport[port].read_port(port)));
    else
        return ECMD_ERR_PARSE_ERROR;

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
      return ECMD_FINAL(print_port(output, len, port, vport[port].read_pin(port)));
    else
        return ECMD_ERR_PARSE_ERROR;

}



#ifdef NAMED_PIN_SUPPORT
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
    return ECMD_FINAL(snprintf_P(output, len,
                      XOR_LOG(vport[port].read_pin(port) & _BV(pin), !(active_high))
                      ? PSTR("on") : PSTR("off")));
  } else
    return ECMD_ERR_PARSE_ERROR;
}


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

      return ECMD_FINAL(snprintf_P(output, len, on ? PSTR("on") : PSTR("off")));
    } else 
      return ECMD_FINAL(snprintf_P(output, len, PSTR("error: pin is input")));

  } else
    return ECMD_ERR_PARSE_ERROR;
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

      return ECMD_FINAL(snprintf_P(output, len, XOR_LOG(!on, !active_high)
                        ? PSTR("on") : PSTR("off")));
    } else 
      return ECMD_FINAL(snprintf_P(output, len, PSTR("error: pin is input")));

  } else
    return ECMD_ERR_PARSE_ERROR;
}

#endif /* NAMED_PIN_SUPPORT */

