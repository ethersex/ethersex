/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "../config.h"
#include "../debug.h"
#include "../uip/uip.h"
#include "../uip/uip_arp.h"
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../portio.h"
#include "named_pin.h"

#ifdef NAMED_PIN_SUPPORT

#define NAMED_PIN_PGM
const char tester123[] PROGMEM = "led1";
const char tester122[] PROGMEM = "taster";
const char tester121[] PROGMEM = "led2";

const struct PinConfiguration portio_pincfg[] PROGMEM = {
  /* Port  Pin  Input Active_High  name */
  {   0,    1,   1,     0,         tester122},
  {   0,    0,   0,     1,         tester123},
  {   0,    2,   0,     0,         tester121},
  { 255,  255, 255,   255,         NULL}
};

void
named_pin_init(void)
/* {{{ */ {
  /* Parse pin table */
  uint8_t i = 0, port, pin, input;
  while (1) {
    port = pgm_read_byte(&portio_pincfg[i].port);
    if (port == 255)
      break;
    pin = pgm_read_byte(&portio_pincfg[i].pin);

    /* Set Input/Output */
    input = pgm_read_byte(&portio_pincfg[i].input); 
    if (input)
      cfg.options.io_ddr[port] &= ~_BV(pin);
    else
      cfg.options.io_ddr[port] |= _BV(pin);

    /* If input and active low set pullup */
    if ((pgm_read_byte(&portio_pincfg[i].active_high) == 0) && input )
      cfg.options.io[port] |= _BV(pin);

    i++;
  }
 } 
/* }}} */

uint8_t
named_pin_by_name(const char *name) 
/* {{{ */ {
  uint8_t i = 0;
  const char *tmp;
  while (1) {
    tmp = (const char *)pgm_read_word(&portio_pincfg[i].name);
    if (tmp == NULL)
      break;
    if (strcmp_P(name, tmp) == 0)
      return i;
    i++;
  }
  return 255;
}
/* }}} */

uint8_t
named_pin_by_pin(uint8_t port, uint8_t pin)
/* {{{ */ {
  uint8_t i = 0;
  uint8_t t_port, t_pin;
  while (1) {
    t_port = pgm_read_byte(&portio_pincfg[i].port);
    t_pin = pgm_read_byte(&portio_pincfg[i].pin);
    if (t_port == 255)
      return 255;
    if (pin == t_pin && port == t_port)
      return i;
    i++;
  }
  return 255;
}
/* }}} */

#endif

