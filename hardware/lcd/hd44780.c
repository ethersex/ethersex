/*
 * hd44780 driver library
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2016 Michael Brakemeier <michael@brakemeier.de>
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

#include <avr/io.h>
#include <util/delay.h>

#include "config.h"

#include "hd44780.h"

#include "core/debug.h"

#ifdef HD44780_READBACK_SUPPORT
/* Readback requires the RW line */
#define HAVE_HD44780_RW 1

#endif

/* macros for defining the data pins as input or output */
#define _DATA_INPUT(a) \
  PIN_CLEAR(HD44780_D ## a); \
  DDR_CONFIG_IN(HD44780_D ## a);

#define DATA_INPUT() \
  do { \
    _DATA_INPUT(4); \
    _DATA_INPUT(5); \
    _DATA_INPUT(6); \
    _DATA_INPUT(7); \
  } while (0)

#define DATA_OUTPUT() \
  do { \
    DDR_CONFIG_OUT(HD44780_D4); \
    DDR_CONFIG_OUT(HD44780_D5); \
    DDR_CONFIG_OUT(HD44780_D6); \
    DDR_CONFIG_OUT(HD44780_D7); \
  } while (0);

#ifdef HD44780_MULTIEN_SUPPORT
#ifndef HD44780_EN1
#define HD44780_EN1 HD44870_EN
#endif
#ifdef HAVE_HD44780_RW
#define CTRL_OUTPUT() \
  do { \
    DDR_CONFIG_OUT(HD44780_EN1); \
    DDR_CONFIG_OUT(HD44780_EN2); \
    DDR_CONFIG_OUT(HD44780_RS); \
    DDR_CONFIG_OUT(HD44780_RW); \
  } while (0);
#else
#define CTRL_OUTPUT() \
  do { \
    DDR_CONFIG_OUT(HD44780_EN1); \
    DDR_CONFIG_OUT(HD44780_EN2); \
    DDR_CONFIG_OUT(HD44780_RS); \
  } while (0);
#endif
// #endif
#else /* not HD44780_MULTIEN_SUPPORT */
#ifdef HAVE_HD44780_RW
#define CTRL_OUTPUT() \
  do { \
    DDR_CONFIG_OUT(HD44780_EN1); \
    DDR_CONFIG_OUT(HD44780_RS); \
    DDR_CONFIG_OUT(HD44780_RW); \
  } while (0);
#else
#define CTRL_OUTPUT() \
  do { \
    DDR_CONFIG_OUT(HD44780_EN1); \
    DDR_CONFIG_OUT(HD44780_RS); \
  } while (0);
#endif
#endif

uint8_t noinline
hd44780_clock_rw(uint8_t read, uint8_t en)
{
  uint8_t data = 0;

  /* set EN high, wait for more than 450 ns */
#ifdef HD44780_MULTIEN_SUPPORT
  if (en == 1)
    PIN_SET(HD44780_EN1);
  else if (en == 2)
    PIN_SET(HD44780_EN2);

#else
  PIN_SET(HD44780_EN1);
#endif
  /* make sure that we really wait for more than 450 ns... */
  _delay_us(1);

  /* read data, if requested.  data pins must be configured as input! */
  if (read)
  {
    if (PIN_HIGH(HD44780_D4))
      data |= _BV(0);
    if (PIN_HIGH(HD44780_D5))
      data |= _BV(1);
    if (PIN_HIGH(HD44780_D6))
      data |= _BV(2);
    if (PIN_HIGH(HD44780_D7))
      data |= _BV(3);
  }

  /* set EN low */

#ifdef HD44780_MULTIEN_SUPPORT
  if (en == 1)
    PIN_CLEAR(HD44780_EN1);
  else if (en == 2)
    PIN_CLEAR(HD44780_EN2);
#else
  PIN_CLEAR(HD44780_EN1);
#endif
  return data;
}

void noinline
hd44780_output_4bit(uint8_t rs, uint8_t nibble, uint8_t en)
{
/* switch to write operation and set rs */
#ifdef HAVE_HD44780_RW
  PIN_CLEAR(HD44780_RW);
#endif
  PIN_CLEAR(HD44780_RS);
  if (rs)
    PIN_SET(HD44780_RS);

  /* compute data bits */
  PIN_CLEAR(HD44780_D4);
  PIN_CLEAR(HD44780_D5);
  PIN_CLEAR(HD44780_D6);
  PIN_CLEAR(HD44780_D7);
  if (nibble & _BV(0))
    PIN_SET(HD44780_D4);
  if (nibble & _BV(1))
    PIN_SET(HD44780_D5);
  if (nibble & _BV(2))
    PIN_SET(HD44780_D6);
  if (nibble & _BV(3))
    PIN_SET(HD44780_D7);

  /* set bits in mask, and delete bits not in mask */
  DATA_OUTPUT();

  /* toggle EN */
  hd44780_clock_write(en);
}

#ifdef HD44780_READBACK_SUPPORT
uint8_t noinline
hd44780_input_4bit(uint8_t rs, uint8_t en)
{
  /* configure data pins as input */
  DATA_INPUT();

  /* set write bit */
  PIN_SET(HD44780_RW);

  /* set rs, if given */
  PIN_CLEAR(HD44780_RS);
  if (rs)
    PIN_SET(HD44780_RS);

  uint8_t data = hd44780_clock_read(en);

  /* reconfigure data pins as output */
  DATA_OUTPUT();

  /* delete RW bit */
  PIN_CLEAR(HD44780_RW);

  return data;
}
#endif

#ifdef HD44780_BACKLIGHT_SUPPORT
void
hd44780_backlight(uint8_t state)
{
  hd44780_backlight_state = state;

#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    PIN_CLEAR(HD44780_BL);
  else
    PIN_SET(HD44780_BL);
#else
  if (hd44780_backlight_state)
    PIN_SET(HD44780_BL);
  else
    PIN_CLEAR(HD44780_BL);
#endif
}
#endif

void noinline
hd44780_hw_init(void)
{
  /* init io pins */
  CTRL_OUTPUT();

  PIN_CLEAR(HD44780_RS);
#ifndef HD44780_MULTIEN_SUPPORT
  PIN_CLEAR(HD44780_EN1);
#endif
#ifdef HD44780_MULTIEN_SUPPORT
  PIN_CLEAR(HD44780_EN1);
  PIN_CLEAR(HD44780_EN2);
#endif
#ifdef HAVE_HD44780_RW
  PIN_CLEAR(HD44780_RW);
#endif
  PIN_CLEAR(HD44780_D4);
  PIN_CLEAR(HD44780_D5);
  PIN_CLEAR(HD44780_D6);
  PIN_CLEAR(HD44780_D7);
  DATA_OUTPUT();

#ifdef HD44780_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state)
    PIN_CLEAR(HD44780_BL);
  else
    PIN_SET(HD44780_BL);
#else
  if (hd44780_backlight_state)
    PIN_SET(HD44780_BL);
  else
    PIN_CLEAR(HD44780_BL);
#endif
#endif
}
