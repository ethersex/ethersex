/*
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "protocols/syslog/syslog.h"
#include "ps2.h"
#include "core/debug.h"

#ifdef PS2_GERMAN_LAYOUT
  #define LC(a,b) (b)
#else
  #define LC(a,b) (a)
#endif


static volatile uint8_t bitcount;
static volatile uint8_t data;
static volatile uint8_t is_up;
static volatile uint8_t parity;
static volatile uint8_t timeout;
static volatile uint8_t leds = 0;

uint8_t send_next = 0;

struct key_press key;
struct key_press ps2_key_cache[5];

/* This is the keymap
 * every byte from the keyboard is connected to a character
 * the LC macro is used to generate german and us layouts 
 */
static uint8_t PROGMEM keycodes[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 00-07 */
  0x00,0x00,0x00,0x00,0x00,0x00, LC('`','^'),0x00, /* keycode 08-0f */
  0x00,0x00,0x00,0x00,0x00, 'q', '1',0x00, /* keycode 10-17 */
  0x00,0x00, LC('z','y'), 's', 'a', 'w', '2',0x00, /* keycode 18-1f */
  0x00, 'c', 'x', 'd', 'e', '4', '3',0x00, /* keycode 20-27 */
  0x00, ' ', 'v', 'f', 't', 'r', '5',0x00, /* keycode 28-2f */
  0x00, 'n', 'b', 'h', 'g', LC('y','z'), '6',0x00, /* keycode 30-37 */
  0x00,0x00, 'm', 'j', 'u', '7', '8',0x00, /* keycode 38-3f */
  0x00, ',', 'k', 'i', 'o', '0', '9',0x00, /* keycode 40-47 */
  0x00, '.', LC('/','-'), 'l',LC(';',0xF6), 'p',LC('-',0xDF),0x00, /* keycode 48-4f */
  0x00,0x00,LC('\'',0xE4),0x00,LC('[',0xFC),LC('=','\''),0x00,0x00, /* keycode 50-57 */
  0x00,0x00,0x00, LC(']','+'),0x00, LC('\\','#'),0x00,0x00, /* keycode 58-5f */
  0x00, '<',0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 60-67 */
  0x00, '1',0x00, '4', '7',0x00,0x00,0x00, /* keycode 68-6f */
   '0', ',', '2', '5', '6', '8',0x00,0x00, /* keycode 70-77 */
  0x00, '+', '3', '-', '*', '9',0x00,0x00, /* keycode 78-7f	*/
};
/* And the keymap with pressed shift key */
static uint8_t PROGMEM keycodes_shift[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* keycode 00-07 */
  0x00,0x00,0x00,0x00,0x00,0x00,LC('~',0x00),0x00, /* keycode 08-0f */
  0x00,0x00,0x00,0x00,0x00, 'Q', '!',0x00, /* keycode 10-17 */
  0x00,0x00, LC('Z','Y'), 'S', 'A', 'W', LC('@','"'),0x00, /* keycode 18-1f */
  0x00, 'C', 'X', 'D', 'E', '$', LC('#','@'),0x00, /* keycode 20-27 */
  0x00, ' ', 'V', 'F', 'T', 'R', '%',0x00, /* keycode 28-2f */
  0x00, 'N', 'B', 'H', 'G', LC('Y','Z'), LC('^','&'),0x00, /* keycode 30-37 */
  0x00,0x00, 'M', 'J', 'U', LC('&','/'), LC('*','('),0x00, /* keycode 38-3f */
  0x00, LC('<',';'), 'K', 'I', 'O', LC(')','='), LC('(',')'),0x00, /* keycode 40-47 */
  0x00, LC('>',':'), LC('?','_'), 'L',LC(':',0xD6), 'P', LC('_', '?'),0x00, /* keycode 48-4f */
  0x00,0x00,LC('"',0xC4),0x00,LC('{',0xDC), LC('+','`'),0x00,0x00, /* keycode 50-57 */
  0x00,0x00,0x00, LC('}','*'),0x00,LC('|','\''),0x00,0x00, /* keycode 58-5f */
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
  key.num = 0;

#ifdef ps2_configure_pcint
  /* Enable the interrupt for the clock pin, because the keyboard is clocking
   * us, not vice versa 
   */
  ps2_configure_pcint ();
#elif defined(HAVE_PS2_INT)
  /* Initialize "real" Interrupt */
  _EIMSK |= _BV(PS2_INT_PIN);
  _EICRA = (_EICRA & ~PS2_INT_ISCMASK) | PS2_INT_ISC;
#endif

  DDR_CONFIG_IN(PS2_DATA);
  DDR_CONFIG_IN(PS2_CLOCK);
  PIN_CLEAR(PS2_DATA);
  PIN_CLEAR(PS2_CLOCK);
}

void
ps2_periodic(void)
{
  /* when we loose a bit we are out of sync, so we want a timeout for
   * recieving bytes
   */
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
  uint8_t sreg = SREG; cli();
  uint8_t i = 11;
  DDR_CONFIG_OUT(PS2_DATA);
  DDR_CONFIG_OUT(PS2_CLOCK);
  /* > 100 us clock low */
  PIN_CLEAR(PS2_CLOCK);
  while (i--)
    _delay_us(10);

  /* data low */
  PIN_CLEAR(PS2_DATA);

  /* clock high */
  PIN_SET(PS2_CLOCK);

  /* Wait until clock is low again */
  DDR_CONFIG_IN(PS2_CLOCK);

  while (PIN_HIGH(PS2_CLOCK));
  uint8_t parity = 1;
  bitcount = 0;
  while(bitcount < 9) {
    /* Parity */
    if (bitcount == 8) {
      PIN_CLEAR(PS2_DATA);
      if (parity)
        PIN_SET(PS2_DATA);
    }
    /* data */
    else {
      PIN_CLEAR(PS2_DATA);
      if (byte & 0x01) {
        PIN_SET(PS2_DATA);
        parity ^= 1;
      }
      byte >>= 1;
    }
    /* The keyboard generates the clock */
    while (!PIN_HIGH(PS2_CLOCK));
    while (PIN_HIGH(PS2_CLOCK));
    bitcount++;
  }
  DDR_CONFIG_IN(PS2_DATA);
  PIN_CLEAR(PS2_DATA);
  PIN_CLEAR(PS2_CLOCK);

  /* Wait for the ack from the keyboard */
  while (PIN_HIGH(PS2_DATA));
  while (PIN_HIGH(PS2_CLOCK));

  SREG = sreg;
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

  case KEY_LIN: /* The LIN key is also known ans windos key */
    key.lin = 0;
    break;

  case KEY_NUM_LOCK: /* num lock toggles the num lock flag */
    key.num ^= 1;
    leds ^= PS2_NUM_LOCK_LED; 
    ps2_send_byte(PS2_SET_LED);
    ps2_send_byte(leds);
    key.lin = 0;
    break;

  case KEY_CAPS_LOCK:
    key.shift = 1;
    leds |= PS2_CAPS_LOCK_LED; 
    ps2_send_byte(PS2_SET_LED);
    ps2_send_byte(leds);
    break;

  default:
    /* put the decoded data into key cache */
    key.keycode = keycode;
    key.data = key.shift 
      ? pgm_read_byte(&keycodes_shift[keycode])
      : pgm_read_byte(&keycodes[keycode]);
 
    memmove(&ps2_key_cache[1], &ps2_key_cache[0], 
            sizeof(ps2_key_cache) - sizeof(*ps2_key_cache));
    memcpy(&ps2_key_cache[0], &key, sizeof(*ps2_key_cache));

    debug_printf("Key: %x %c\n", key.keycode, key.data); 

#ifdef SYSLOG_SUPPORT
    /* For debugging purposes we send the keycode via syslog */
    if (key.extended && keycode == 0x6c)
      syslog_sendf_P(PSTR("HOME"));

    syslog_sendf_P(PSTR("Key: %x %c"), data, key.shift
                 ? pgm_read_byte(&keycodes_shift[keycode])
                 : pgm_read_byte(&keycodes[keycode]));
#endif
    break;
  }
}

ISR(PS2_VECTOR) 
{
  if (! PIN_HIGH(PS2_CLOCK)) {
    /* Start the timeout to 20ms - 40ms */
    if (bitcount == 11) 
      timeout = 2;

    /* the bits 10 to 3 are our data byte */
    if (bitcount < 11 && bitcount > 2) {
      data >>= 1;
      if ( PIN_HIGH(PS2_DATA)) {
        data |= 0x80;
        parity ^= 1;
      }
    } else if (bitcount == 2) {
      /* check the parity, and drop the byte if it doesn't match */
      if (((PIN_HIGH(PS2_DATA)) ^ (parity << PIN_NR(PS2_DATA))) != 0) {
        data = 0;
      }
    }

    if (--bitcount == 0) {
      /* the is up flag is set, when the button was released, after the is_up
       * byte, the keycode is repeated once again 
       */
      if (is_up) {
        decode_key(data);
        is_up = 0;
      } else if (data == 0xF0 && ! is_up) 
        is_up = 1;
      else {
        /* Some codes must be processed, while they are hold down */
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

        /* The NUMPAD is attached via a extended keymap, we use a flag for
         * this
         */
        case 0xE0:
          key.extended = 1;
          break;

        default:
          key.extended = 0;
          break;
        }
      }
      parity = 1;
      bitcount = 11;
    }
  }
}

/*
  -- Ethersex META --
  header(hardware/input/ps2/ps2.h)
  init(ps2_init)
  timer(1, ps2_periodic())
*/
