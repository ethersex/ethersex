/* Library for using an lcd in 4 bit mode
 
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

//#include <avr/io.h>
//#include <avr/pgmspace.h>

#include "lcd.h"
#include "../lcd/hd44780.h"

#ifdef HD44780_SUPPORT

void 
lcd_clear(void)
{
   hd44780_clear();
}
 
void 
lcd_home(void)
{
   hd44780_home();
}

void
lcd_print(char *string)
{
  char *p = string;
  while (*p) {
    hd44780_put(*p, NULL);
    p++;
  }
}

#else /* mein eigener lcd support */

void 
lcd_send(uint8_t data, uint8_t direction)
{
   uint8_t temp = data;
   
   PORT_CHAR(LCD_RS_PORT) &= ~_BV(LCD_RS_PIN);
   if (direction) 
     PORT_CHAR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
 
   data >>= 4;
   data &= 0x0F;
   PORT_CHAR(LCD_DATA_PORT) &= ~(15 << LCD_DATA_PORT_SHIFT);
   PORT_CHAR(LCD_DATA_PORT) |= data << LCD_DATA_PORT_SHIFT;
   lcd_enable();
 
   temp &= 0x0F;
   PORT_CHAR(LCD_DATA_PORT) &= ~(15 << LCD_DATA_PORT_SHIFT);
   PORT_CHAR(LCD_DATA_PORT) |= temp << LCD_DATA_PORT_SHIFT;
   lcd_enable();
   _delay_us(700);
}

void 
lcd_enable(void)
{
  PORT_CHAR(LCD_ENABLE_PORT) |= _BV(LCD_ENABLE_PIN);
   asm("nop");
   asm("nop");
   asm("nop");
  PORT_CHAR(LCD_ENABLE_PORT) &= ~_BV(LCD_ENABLE_PIN);
}

void lcd_init(void)
{
   uint8_t count = 50;

   DDR_CHAR(LCD_DATA_PORT) |= 15 << LCD_DATA_PORT_SHIFT;
   DDR_CHAR(LCD_ENABLE_PORT) |= _BV(LCD_ENABLE_PIN);
   DDR_CHAR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
 
   while(count)
   {
      _delay_ms(5); 
      count--;
   }
 
   // muss 3mal hintereinander gesendet werden zur Initialisierung
   PORT_CHAR(LCD_DATA_PORT) &= ~(15 << LCD_DATA_PORT_SHIFT);
   PORT_CHAR(LCD_DATA_PORT) |= 0x03 << LCD_DATA_PORT_SHIFT; 
   lcd_enable();
   _delay_ms(5);
   lcd_enable();
   _delay_ms(5);
   lcd_enable();
 
   // 4bit-Modus einstellen
   PORT_CHAR(LCD_DATA_PORT) &= ~(15 << LCD_DATA_PORT_SHIFT);
   PORT_CHAR(LCD_DATA_PORT) |= 0x02 << LCD_DATA_PORT_SHIFT; 
   lcd_enable();
   _delay_ms(15);
 
#ifdef LCD_TWO_LINES
   // 4Bit / 2 line / 5x8
   lcd_command(0x28);  // 0b00101000        
   lcd_command(0x28);  // 0b00101000        
   lcd_command(0x28);  // 0b00101000        
#else
   // 4Bit / 1 line / 5x8
   lcd_command(0x20);
   lcd_command(0x20);
   lcd_command(0x20);
#endif

   // Display ein / Cursor aus / kein Blinken
   lcd_command(0x0C);  // 0b00001100        
 
   // inkrement / kein Scrollen
   lcd_command(0x04);  // 0b00000100

   lcd_clear();
   lcd_home();
}

void 
lcd_clear(void)
{
   lcd_command(0x01);
   _delay_ms(10);
}
 
void 
lcd_home(void)
{
   lcd_command(0x02);
   _delay_ms(10);
}

void
lcd_print(char *string)
{
  char *p = string;
  while (*p) {
    lcd_data(*p);
    p++;
  }
}

#if 0
void 
lcd_print_p(const char *progmem_s)
{
  uint8_t c;
  while (( c = pgm_read_byte(progmem_s++)))
    lcd_data(c);
}
#endif

#if 0
void
lcd_define_char(uint8_t n_char, uint8_t *data)
{
  if (n_char > 7) return;
  // set cgram pointer to char number n
  lcd_command( 0x40 | n_char * 8);
  n_char = 0;
  while (n_char < 8) {
    // send the data to lcd into cgram
    lcd_data(*(data + n_char));
    n_char++;
  }
  // set pointer back to Home
  lcd_command(0x80);
}
#endif

#if 0
void
lcd_define_char_p(uint8_t n_char, const uint8_t *data)
{
  if (n_char > 7) return;
  // set cgram pointer to char number n
  lcd_command( 0x40 | n_char * 8);
  n_char = 0;
  while (n_char < 8) {
    // send the data to lcd into cgram
    lcd_data(pgm_read_byte(data + n_char));
    n_char++;
  }
  // set pointer back to Home
  lcd_command(0x80);
}
#endif

#endif /* mein eigener lcd support */
