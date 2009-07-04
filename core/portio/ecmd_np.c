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

#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "core/bit-macros.h"
#include "core/portio/named_pin.h"
#include "core/portio/portio.h"
#include "protocols/ecmd/ecmd-base.h"

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


/*
  -- Ethersex META --
  ecmd_feature(pin_get, "pin get", NAME, Read and display the status of pin NAME.)
  ecmd_feature(pin_set, "pin set", NAME STATUS, Set the status of pin NAME to STATUS.)
  ecmd_feature(pin_toggle, "pin toggle", NAME, Toggle the status of pin NAME.)
*/
