/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../bit-macros.h"
#include "../config.h"
#include "../syslog/syslog.h"
#include "ps2.h"

#ifdef PS2_SUPPORT

static volatile uint8_t bitcount;
static volatile uint8_t data;
static volatile uint8_t is_up;
static volatile uint8_t parity;
static volatile uint8_t timeout;
static volatile uint8_t leds = PS2_NUM_LOCK_LED;

uint8_t send_next = 0;

struct key_press key;

static uint8_t PROGMEM keycodes[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 00-07 */
  0x00,0x00,0x00,0x00,0x00,0x00, '^',0x00, /* keycode 08-0f */
  0x00,0x00,0x00,0x00,0x00, 'q', '1',0x00, /* keycode 10-17 */
  0x00,0x00, 'y', 's', 'a', 'w', '2',0x00, /* keycode 18-1f */
  0x00, 'c', 'x', 'd', 'e', '4', '3',0x00, /* keycode 20-27 */
  0x00, ' ', 'v', 'f', 't', 'r', '5',0x00, /* keycode 28-2f */
  0x00, 'n', 'b', 'h', 'g', 'z', '6',0x00, /* keycode 30-37 */
  0x00,0x00, 'm', 'j', 'u', '7', '8',0x00, /* keycode 38-3f */
  0x00, ',', 'k', 'i', 'o', '0', '9',0x00, /* keycode 40-47 */
  0x00, '.', '/', 'l',0x00, 'p',0x00,0x00, /* keycode 48-4f */
  0x00,0x00,0x00,0x00,0x00,'\'',0x00,0x00, /* keycode 50-57 */
  0x00,0x00,0x00, '+',0x00, '#',0x00,0x00, /* keycode 58-5f */
  0x00, '<',0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 60-67 */
  0x00, '1',0x00, '4', '7',0x00,0x00,0x00, /* keycode 68-6f */
   '0', ',', '2', '5', '6', '8',0x00,0x00, /* keycode 70-77 */
  0x00, '+', '3', '-', '*', '9',0x00,0x00, /* keycode 78-7f	*/
};

static uint8_t PROGMEM keycodes_shift[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 00-07 */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 08-0f */
  0x00,0x00,0x00,0x00,0x00, 'Q', '!',0x00, /* keycode 10-17 */
  0x00,0x00, 'Y', 'S', 'A', 'W', '"',0x00, /* keycode 18-1f */
  0x00, 'C', 'X', 'D', 'E', '$', '@',0x00, /* keycode 20-27 */
  0x00, ' ', 'V', 'F', 'T', 'R', '%',0x00, /* keycode 28-2f */
  0x00, 'N', 'B', 'H', 'G', 'Z', '&',0x00, /* keycode 30-37 */
  0x00,0x00, 'M', 'J', 'U', '/', '(',0x00, /* keycode 38-3f */
  0x00, ';', 'K', 'I', 'O', '=', ')',0x00, /* keycode 40-47 */
  0x00, ':', '/', 'L',0x00, 'P', '?',0x00, /* keycode 48-4f */
  0x00,0x00,0x00,0x00,0x00, '`',0x00,0x00, /* keycode 50-57 */
  0x00,0x00,0x00, '*',0x00,'\'',0x00,0x00, /* keycode 58-5f */
  0x00, '>',0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 60-67 */
  0x00, '1',0x00, '4', '7',0x00,0x00,0x00, /* keycode 68-6f */
   '0',0x00, '2', '5', '6', '8',0x00,0x00, /* keycode 70-77 */
  0x00, '+', '3', '-', '*', '9',0x00,0x00, /* keycode 78-7f	*/
};

void
ps2_init(void) 
{
  bitcount = 11;
  parity = 1;

  PCICR |= _BV(PS2_PCIE);
  PS2_PCMSK |= _BV(PS2_CLOCK_PIN);

  PS2_DDR &= ~(_BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN));
  PS2_PORT &= ~(_BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN));

}

void
ps2_periodic(void)
{
  if(timeout--)
    if(timeout == 0) {
      /* Reset the reciever */
      parity = 1;
      bitcount = 11;
    }
}

void
ps2_send_byte(uint8_t byte)
{
  cli();
  uint8_t i = 11;
  PS2_DDR |= _BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN);
  /* > 100 us clock low */
  PS2_PORT &= ~_BV(PS2_CLOCK_PIN);
  while (i--)
    _delay_us(10);
  /* data low */
  PS2_PORT &= ~_BV(PS2_DATA_PIN);
  /* clock high */
  PS2_PORT |= _BV(PS2_CLOCK_PIN);
  /* Wait until clock is low again */
  PS2_DDR &= ~_BV(PS2_CLOCK_PIN);
  while (PS2_PIN & _BV(PS2_CLOCK_PIN));
  uint8_t parity = 1;
  bitcount = 0;
  while(bitcount < 9) {
    /* Parity */
    if (bitcount == 8) {
      PS2_PORT &= ~_BV(PS2_DATA_PIN);
      if (parity)
        PS2_PORT |= _BV(PS2_DATA_PIN);
    }
    /* data */
    else {
      PS2_PORT &= ~_BV(PS2_DATA_PIN);
      if (byte & 0x01) {
        PS2_PORT |= _BV(PS2_DATA_PIN);
        parity ^= 1;
      }
      byte >>= 1;
    }
    /* The keyboard generates the clock */
    while (!(PS2_PIN & _BV(PS2_CLOCK_PIN)));
    while (PS2_PIN & _BV(PS2_CLOCK_PIN));
    bitcount++;
  }

  PS2_DDR &= ~_BV(PS2_DATA_PIN);
  PS2_PORT &= ~(_BV(PS2_DATA_PIN) | _BV(PS2_CLOCK_PIN));
  /* Wait for the ack from the keyboard */
  while (PS2_PIN & _BV(PS2_DATA_PIN));
  while (PS2_PIN & _BV(PS2_CLOCK_PIN));

  sei();
}

static void
decode_key(uint8_t keycode) 
{
  switch(keycode) {
  case KEY_ALT:
    key.alt = 0;
    break;
  case KEY_CTRL:
    key.ctrl = 0;
    break;
  case KEY_SHIFT_RIGHT:
  case KEY_SHIFT_LEFT:
    key.shift = 0;
    leds &= ~PS2_CAPS_LOCK_LED; 
    ps2_send_byte(PS2_SET_LED);
    ps2_send_byte(leds);
    break;
  case KEY_LIN:
    key.lin = 0;
    break;
  case KEY_CAPS_LOCK:
    key.shift = 1;
    leds |= PS2_CAPS_LOCK_LED; 
    ps2_send_byte(PS2_SET_LED);
    ps2_send_byte(leds);
    break;
  default:
    syslog_sendf("Key: %x %c", data, key.shift 
                 ? pgm_read_byte(&keycodes_shift[data])
                 : pgm_read_byte(&keycodes[data]));
    break;
  }
}

SIGNAL(PS2_INTERRUPT) 
{
  if (!(PS2_PIN & _BV(PS2_CLOCK_PIN))) {
    /* Start the timeout to 20ms - 40ms */
    if (bitcount == 1) 
      timeout = 2;

    if (bitcount < 11 && bitcount > 2) {
      data >>= 1;
      if ( PS2_PIN & _BV(PS2_DATA_PIN)) {
        data |= 0x80;
        parity ^= 1;
      }
    } else if (bitcount == 2) {
      if (((PS2_PIN & _BV(PS2_DATA_PIN)) ^ (parity << PS2_DATA_PIN)) != 0) {
        data = 0;
      }
    }

    if (--bitcount == 0) {
      if (is_up) {
        decode_key(data);
        is_up = 0;
      } else if (data == 0xF0 && ! is_up) 
        is_up = 1;
      else {
        switch(data) {
        case KEY_ALT:
          key.alt = 1;
          break;
        case KEY_CTRL:
          key.ctrl = 1;
          break;
        case KEY_SHIFT_RIGHT:
        case KEY_SHIFT_LEFT:
          key.shift = 1;
          break;
        case KEY_LIN:
          key.lin = 1;
          break;
        }
      }
      parity = 1;
      bitcount = 11;
    }
  }
}

#endif
