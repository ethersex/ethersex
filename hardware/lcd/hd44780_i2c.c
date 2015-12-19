/*
 * hd44780 driver library fur use of hd44780 with pcf8574 i2c expander
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * (c) by Alexander Tuschen <atuschen@web.de>
 * (c) by Mirko Taschenberger <mirkiway at gmx.de>
 * (c) 2014 by Daniel Lindner <daniel.lindner at gmx.de>
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

#include "hd44780.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pcf8574x.h"

/* global variables */
uint8_t lcd_data;


uint8_t noinline
clock_rw(uint8_t read, uint8_t en)
{
  uint8_t data = 0;

#ifdef HD44780_MULTIENSUPPORT
  uint8_t enPin = 0;
  if (en == 1) {
    enPin = HD44780_PCF8574x_EN1;
  } else if (en == 2) {
    enPin = HD44780_PCF8574x_EN2;
  } else {
    debug_printf("#### unknown EN!\n");
  }
#else
  uint8_t enPin = HD44780_PCF8574x_EN;
#endif

  if (read)
  {                             /* Datenbyte an PCF senden, DBx high zum lesen, sollte vor EN high Flanke geschehen */
    lcd_data |= (_BV(HD44780_PCF8574x_DB4) |
                 _BV(HD44780_PCF8574x_DB5) |
                 _BV(HD44780_PCF8574x_DB6) | _BV(HD44780_PCF8574x_DB7));
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* EN setzen */
    lcd_data |= _BV(enPin);
    /* Datenbyte an PCF senden, DBx high zum lesen, EN uebertragen */
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* Datenbyte von PCF lesen, daten_nibble maskiert */
    uint8_t i2c_read_data;
    i2c_read_data = i2c_pcf8574x_read(HD44780_PCF8574x_ADR);
    if (i2c_read_data & _BV(HD44780_PCF8574x_DB4))
      data |= _BV(0);
    if (i2c_read_data & _BV(HD44780_PCF8574x_DB5))
      data |= _BV(1);
    if (i2c_read_data & _BV(HD44780_PCF8574x_DB6))
      data |= _BV(2);
    if (i2c_read_data & _BV(HD44780_PCF8574x_DB7))
      data |= _BV(3);
#ifdef DEBUG_HD44780_I2C
    debug_printf("I2C-LCD-R: read data 0x%X  raw(0x%X) [(%u) (0x%X)]\n", data, i2c_read_data, enPin, HD44780_PCF8574x_ADR);
#endif
  }
  else
  {                             /* Datenbyte an PCF senden, muss vor EN high Flanke geschehen */
#ifdef DEBUG_HD44780_I2C
    debug_printf("I2C-LCD-W: (%u) 0x%X\n",enPin, lcd_data);
#endif
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
    /* EN setzen */
    lcd_data |= _BV(enPin);
    /* Datenbyte an PCF senden, EN status uebertragen */
    i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
  }

  /* EN loeschen */
  lcd_data &= ~(_BV(enPin));
  /* Datenbyte erneut senden, EN status uebertragen */
  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);

  return data;
}

void noinline
output_nibble(uint8_t rs, uint8_t nibble, uint8_t en)
{
  /* compute data bits */
  lcd_data = 0;
  if (nibble & _BV(0))
    lcd_data |= _BV(HD44780_PCF8574x_DB4);
  if (nibble & _BV(1))
    lcd_data |= _BV(HD44780_PCF8574x_DB5);
  if (nibble & _BV(2))
    lcd_data |= _BV(HD44780_PCF8574x_DB6);
  if (nibble & _BV(3))
    lcd_data |= _BV(HD44780_PCF8574x_DB7);

  /* Wenn rs==1, dann RS setzen */
  if (rs)
    lcd_data |= _BV(HD44780_PCF8574x_RS);

#ifdef HD44780_BACKLIGHT_SUPPORT
  /* backlight status falls vorhanden uebernehmen */
  if (back_light)
    lcd_data |= _BV(HD44780_PCF8574x_BL);
  else
    lcd_data &= ~(_BV(HD44780_PCF8574x_BL));
#endif

  /* toggle EN, daten uebertragen */
  clock_write(en);
}

#ifdef HD44780_READBACK
uint8_t noinline
input_nibble(uint8_t rs, uint8_t en)
{
  /* WR setzen = lesen */
  lcd_data |= _BV(HD44780_PCF8574x_WR);

  /* Wenn rs gesetzt, */
  if (rs)                       /* dann RS setzen */
    lcd_data |= _BV(HD44780_PCF8574x_RS);
  else                          /* sonst RS loeschen */
    lcd_data &= ~(_BV(HD44780_PCF8574x_RS));

  /* toggle EN, daten abholen */
  uint8_t data = clock_read(en);

  /* WR loeschen = schreiben */
  lcd_data &= ~(_BV(HD44780_PCF8574x_WR));

  return data;
}
#endif

#ifdef HD44780_BACKLIGHT_SUPPORT
void
hd44780_backlight(uint8_t state)
{
  back_light = state;

  /* backlight status falls vorhanden uebernehmen */
  if (back_light)
    lcd_data |= _BV(HD44780_PCF8574x_BL);
  else
    lcd_data &= ~(_BV(HD44780_PCF8574x_BL));
  /* backlight status direkt uebertragen */
  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
}
#endif

void noinline
hd44780_hw_init(void)
{
  /* alle ausgaenge auf 0 setzen */
  lcd_data = 0;
#ifdef HD44780_BACKLIGHT_INV
  back_light = 1;
  lcd_data |= _BV(HD44780_PCF8574x_BL);
#endif

  i2c_pcf8574x_set(HD44780_PCF8574x_ADR, lcd_data);
}
