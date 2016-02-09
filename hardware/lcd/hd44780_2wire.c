/*
 * hd44780 driver library fur use of hd44780 with 74hc4094 expander
 * aka 2-Draht-LCD as used by the ATM18 project
 * http://www.cczwei.de/atm18_downloads/071148-D%20CC2-AVR-board-2.pdf
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2009 by Albrecht Petzold <albrecht.petzold@web.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 3 or
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

#include "hd44780.h"
#include "config.h"

#ifdef DEBUG_HD44780
# include "core/debug.h"
# define HD44780DEBUG(a...)  debug_printf("HD44780: " a)
#else
# define HD44780DEBUG(a...) do { } while(0)
#endif	/* DEBUG_HD44780 */

/* macros for defining the data pins as input or output */
#define DATA_OUTPUT() do { \
			  DDR_CONFIG_OUT(HD44780_2WIRE_D); \
			 } while (0);

#define CTRL_OUTPUT() do { \
			  DDR_CONFIG_OUT(HD44780_2WIRE_CLK); \
			 } while (0);

/* other macros */
#define noinline __attribute__((noinline))

// time for high level of clocking signal
#define LCD_DELAY() _delay_us(1)

/* own prototypes */
static noinline void shift_nibble_out(uint8_t data);
void clear_shift_register(void);


void noinline shift_nibble_out(uint8_t data)
{
// 	HD44780DEBUG("try to send: %X\n", data);
	uint8_t mask = 0x08;

	for (uint8_t n = 4; n > 0; n--) // Do this for 4 databits
	{
// 		HD44780DEBUG("send %i bit: %X\n", n, data & mask );
		if (data & mask) // Set or reset data signal
		  PIN_SET(HD44780_2WIRE_D);
		else
		  PIN_CLEAR(HD44780_2WIRE_D);
		PIN_SET(HD44780_2WIRE_CLK);
		LCD_DELAY();
		PIN_CLEAR(HD44780_2WIRE_CLK);
		mask >>= 1; // Shift right compare mask
	}

	// Clear data line, then do the last shift, Q7=1
	// Always clear data line before the last shift
	PIN_CLEAR(HD44780_2WIRE_D); 

	PIN_SET(HD44780_2WIRE_CLK);
	LCD_DELAY();
	PIN_CLEAR(HD44780_2WIRE_CLK);
}

uint8_t noinline clock_rw(uint8_t read,uint8_t en)
{
	// Now 6 bits are shifted, Q7=1 waiting for the data line to go high
	// for a logical AND, thus a high at E to latch the data at Q5..Q2
	PIN_SET(HD44780_2WIRE_D);
	_delay_us(1); // ensure 450ns high level

	PIN_CLEAR(HD44780_2WIRE_D); // Clear E always
	LCD_DELAY();

	return 0;
}

void noinline output_nibble(uint8_t rs, uint8_t nibble, uint8_t en)
{
	clear_shift_register();
	// Set high level for E at Q7
	PIN_SET(HD44780_2WIRE_D);
	PIN_SET(HD44780_2WIRE_CLK);
	LCD_DELAY();
	PIN_CLEAR(HD44780_2WIRE_CLK);

  // Set level for RS at Q6
	if (rs) //Soll ins Seuer oder Datenregister geschrieben werden?
		PIN_SET(HD44780_2WIRE_D);
	else
		PIN_CLEAR(HD44780_2WIRE_D);
	PIN_SET(HD44780_2WIRE_CLK);
	LCD_DELAY();
	PIN_CLEAR(HD44780_2WIRE_CLK);

  // shift nibble
	shift_nibble_out(nibble);
	
   /* toggle EN */
	clock_write(1);
}

#ifdef HD44780_BACKLIGHT_SUPPORT
void hd44780_backlight(uint8_t state)
{
  /* Don't know what this function is used for so it does nothing ATM.
   * Needs to be fixed from someone with more insight ;-)
   */
    back_light = state;
}
#endif

void noinline hd44780_hw_init(void)
{
	HD44780DEBUG("starte HW init\n");
		// set data direction to output & clear pins
	CTRL_OUTPUT();
	DATA_OUTPUT();

	PIN_CLEAR(HD44780_2WIRE_CLK);
	PIN_CLEAR(HD44780_2WIRE_D);

	_delay_ms(50); // Delay for power up time

	clear_shift_register();
	HD44780DEBUG("HW init finished\n");
}

void clear_shift_register(void)
{
	PIN_CLEAR(HD44780_2WIRE_D);
	for (uint8_t a=7; a>0; a--)
	{
		PIN_SET(HD44780_2WIRE_CLK);
		LCD_DELAY();
		PIN_CLEAR(HD44780_2WIRE_CLK);
		LCD_DELAY();
    }
}
