/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

/*
 * Parts of this file are derived from Open HR20 LCD Driver, which is
 * distributed unter GPLv2+ and
 *
 * Copyright (C) 2008 Dario Carluccio (hr20-at-carluccio-dot-de)
 *		 2009 Thomas Vosshagen (mod. for THERMOTronic) (openhr20-at-vosshagen-dot-com)
 */

#include <util/delay.h>
#include <string.h>

#include "config.h"
#include "hardware/lcd/hr20.h"

#define HR20_LCD_INITIAL_CONTRAST   15

#include "hr20-charset.c"

void
hr20_lcd_init (void)
{
  /* Set all segments (i.e. write 0xFF to LCDDR0..LCDDR19). */
  memset ((void *) &LCDDR0, 0xFF, 20);

  /* Set the initial LCD contrast level */
//  LCDCCR = HR20_LCD_INITIAL_CONTRAST << LCDCC0;

/* Set the initial LCD contrast level
        Nominal drive time  = 300uS default     */
  LCDCCR |= HR20_LCD_INITIAL_CONTRAST;

  /* LCD Control and Status Register B
	- clock source is TOSC1 pin
	- COM0:2 connected
	- SEG0:22 connected */
//  LCDCRB = (1<<LCDCS) | (1<<LCDMUX1) | (1<<LCDPM2)| (1<<LCDPM0);

/* LCD Control and Status Register B
        - clock source is the system clock
        - COM0:2 connected
        - SEG0:22 connected
        - LCD Duty Cycle 1/3    */
  LCDCRB = (1<<LCDMUX1) | (1<<LCDPM2)| (1<<LCDPM0);


  /* LCD Frame Rate Register
	- LCD Prescaler Select N=16       @32.768Hz ->   2048Hz
	- LCD Duty Cycle 1/3 (K=6)       2048Hz / 6 -> 341,33Hz
	- LCD Clock Divider  (D=5)     341,33Hz / 7 ->  47,76Hz */
//  LCDFRR = ((1<<LCDCD2)|(1<<LCDCD1));

/* LCD Frame Rate Register
        - LCD Prescaler Select N=4096           @8Mhz          -->  1953.125Hz
        - LCD Duty Cycle 1/3 (K=6)              1953.125Hz / 6 -->  325.52Hz
        - LCD Clock Divider  (D=5)               325.52Hz / 8  -->  40.69Hz          */

  LCDFRR = (1<<LCDCD2) | (1<<LCDCD1) | (1<<LCDCD0) | (1<<LCDPS2) | (1<<LCDPS1) | (1<<LCDPS0)  ;



  /* LCD Control and Status Register A
	- Enable LCD
	- Set Low Power Waveform */
  LCDCRA = (1<<LCDEN) | (1<<LCDAB);

  /* Wait some time showing all segments, i.e. some kind
     of display test/show activity. */
  for (uint8_t i = 0; i < 100; i ++) {
    wdt_kick ();
    _delay_ms (10);
  }

  /* Clear display (i.e. write zero to LCDDR0..LCDDR19). */
  memset ((void *) &LCDDR0, 0, 20);

//  LCD_SEG_SET (LCD_SEG_MOON);
//  LCD_SEG_SET (LCD_SEG_AUTO);

//  hr20_lcd_putchar (2, HR20_LCD_CHAR_2);
//  hr20_lcd_putchar (1, HR20_LCD_CHAR_3);
//  hr20_lcd_hourbar (23, 5);
}


void
hr20_lcd_putchar (uint8_t pos, uint8_t ch)
{
  pos = pgm_read_byte (&hr20_nf_offsets[pos]);
  ch = pgm_read_byte (&hr20_charset[ch]);

  for (uint8_t i = 0; i < 7; i ++) {
    uint8_t seg = pos + pgm_read_byte (&hr20_seg_offsets[i]);
    if (ch & _BV(i))
      LCD_SEG_SET (seg);
    else
      LCD_SEG_CLEAR (seg);
  }
}


void
hr20_lcd_hourbar (uint8_t start, uint8_t stop)
{
  uint8_t left_portion = 0;

  if (stop < start) {
    left_portion = stop + 1;
    stop = 24;
  }

  for (uint8_t i = 0; i < 24; i ++) {
    uint8_t seg = pgm_read_byte (&hr20_bar_offsets[i]);
    if (i < left_portion || (i >= start && i <= stop))
      LCD_SEG_SET (seg);
    else
      LCD_SEG_CLEAR (seg);
  }
}


/*
  -- Ethersex META --
  header(hardware/lcd/hr20.h)
  init(hr20_lcd_init)
*/
