/*
 * hd44780 driver library fur use of hd44780 with 74hc(t)164 expander
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * (c) Mirko Taschenberger <mirkiway at gmx.de>
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

/* macros for defining the data pins as input or output */
#define DATA_OUTPUT() do { \
			  DDR_CONFIG_OUT(HD44780_SER_D); \
			 } while (0);

#define CTRL_OUTPUT() do { \
			  DDR_CONFIG_OUT(HD44780_SER_EN1); \
			  DDR_CONFIG_OUT(HD44780_SER_CLK); \
			 } while (0);

#define LCD_RS_PIN		0
#define LCD_LIGHT_PIN		7

/* own prototypes */
static noinline void shift_data_out(uint8_t byte);


void noinline
shift_data_out(uint8_t byte)
{
  for (uint8_t a = 8; a > 0; a--)
  {
    if ((byte & (1 << (a - 1))) != 0)
      PIN_SET(HD44780_SER_D);
    else
      PIN_CLEAR(HD44780_SER_D);

    PIN_SET(HD44780_SER_CLK);
    _delay_us(1);
    PIN_CLEAR(HD44780_SER_CLK);
  }
}

uint8_t noinline
hd44780_clock_rw(uint8_t read, uint8_t en)
{
  /* set EN high, wait for more than 450 ns */
  PIN_SET(HD44780_SER_EN1);

  /* make sure that we really wait for more than 450 ns... */
  _delay_us(1);

  /* set EN low */
  PIN_CLEAR(HD44780_SER_EN1);

  return 0;
}

void noinline
hd44780_output_4bit(uint8_t rs, uint8_t nibble, uint8_t en)
{
  uint8_t data = 0;
  //Soll ins Seuer oder Datenregister geschrieben werden?
  if (rs)
    data |= (1 << LCD_RS_PIN);

#ifdef HD44780_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state == 0)
    data |= (1 << LCD_LIGHT_PIN);
#else
  if (hd44780_backlight_state)
    data |= (1 << LCD_LIGHT_PIN);
#endif
#endif

  data |= (nibble & 0x0f) << 3; //Write Nibble

  //Schreiben an das LCD
  shift_data_out(data);

  /* toggle EN */
  hd44780_clock_write(1);
}

#ifdef HD44780_BACKLIGHT_SUPPORT
void
hd44780_backlight(uint8_t state)
{
  uint8_t data = 0;
  hd44780_backlight_state = state;

#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state == 0)
    data |= (1 << LCD_LIGHT_PIN);
#else
  if (hd44780_backlight_state)
    data |= (1 << LCD_LIGHT_PIN);
#endif

  shift_data_out(data);
}
#endif

void noinline
hd44780_hw_init(void)
{
  uint8_t data = 0;

  CTRL_OUTPUT();
  PIN_CLEAR(HD44780_SER_CLK);
  PIN_CLEAR(HD44780_SER_EN1);

  PIN_CLEAR(HD44780_SER_D);
  DATA_OUTPUT();

#ifdef HD44780_BACKLIGHT_SUPPORT
#ifdef HD44780_BACKLIGHT_INV
  if (hd44780_backlight_state == 0)
    data |= (1 << LCD_LIGHT_PIN);
#else
  if (hd44780_backlight_state)
    data |= (1 << LCD_LIGHT_PIN);
#endif
#endif

  shift_data_out(data);
}
