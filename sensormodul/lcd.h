#ifndef _LCD_H
#define _LCD_H
/* Library for using an lcd in 4 bit mode
 *
   Copyright(C) 2007 Christian Dietrich <stettberger@brokenpipe.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

/* Documentation ( i hope it isn't too bad )
Configuration:
Data Ports are PC2-PC6:
 #define LCD_DATA_PORT C
 #define LCD_DATA_PORT_SHIFT 2

Enable Pin on PD4
 #define LCD_ENABLE_PORT D
 #define LCD_ENABLE_PIN 4

RS Pin on PB2
 #define LCD_RS_PORT B
 #define LCD_RS_PIN 2

LCD can display 2 lines:
 #define LCD_TWO_LINES

and then include the lcd.c
 #include "lcd.c"

Initialisation and usage:
 lcd_init();

 lcd_print("Hello World");
 lcd_goto_ddram(LCD_SECOND_LINE);
 lcd_print("LCDs are rocking");

*/

#include <avr/io.h>
#include <avr/pgmspace.h>

#ifndef F_CPU
#define F_CPU 800000UL
#endif
#include <util/delay.h>

#ifndef LCD_DATA_PORT
#define LCD_DATA_PORT D
#endif

#ifndef LCD_DATA_PORT_SHIFT
#define LCD_DATA_PORT_SHIFT 4
#endif

#ifndef LCD_ENABLE_PORT
#define LCD_ENABLE_PORT D
#endif

#ifndef LCD_ENABLE_PIN
#define LCD_ENABLE_PIN 1
#endif

#ifndef LCD_RS_PORT
#define LCD_RS_PORT D
#endif

#ifndef LCD_RS_PIN
#define LCD_RS_PIN 0
#endif

#define DELAY50US() \
           _delay_us(25.0); \
           _delay_us(25.0)


#define _PORT_CHAR(character) PORT ## character
#define PORT_CHAR(character) _PORT_CHAR(character)

#define _DDR_CHAR(character) DDR ## character
#define DDR_CHAR(character) _DDR_CHAR(character)

#define LCD_FIRST_LINE 0x00
#define LCD_SECOND_LINE 0x40

#define LCD_RS_DATA 1
#define LCD_RS_COMMAND 0

#define LCD_TWO_LINES

/* send byte to lcd */
void lcd_send(uint8_t data, uint8_t direction);

/* send databyte to lcd */
#define lcd_data(data) lcd_send((data), LCD_RS_DATA)

/* send command to lcd */
#define lcd_command(command) lcd_send((command), LCD_RS_COMMAND)

/* pulse the enable pin */
void lcd_enable(void);

/* initialize the lcd */
void lcd_init(void);

/* clear the display */
void lcd_clear(void);

/* go to ddram address 0 and disable display shift */
void lcd_home(void);

/* goto special ddram address */
#define lcd_goto_ddram(address) lcd_command(0x80 | (address))

/* shift display left */
#define lcd_shift_left() lcd_command(0x18)

/* shift display right */
#define lcd_shift_right() lcd_command(0x1C)

/* send string to the display */
void lcd_print(char *string);

/* send string to the lcd, read from progmem */
void lcd_print_p(const char *progmem_s);

/* Defining new Characters:
 * There are 8 user definable characters 0x00-0x07. They had to be written on
 * every reset of the terminal, because they are only saved in cgram
 */

/* define the character n_char from ram */
void lcd_define_char(uint8_t n_char, uint8_t *data);

/* define the character n_char from flash */
void lcd_define_char_p(uint8_t n_char, const uint8_t *data);

#endif
