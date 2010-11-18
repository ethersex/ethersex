/*
 * hd44780 driver library fur use of hd44780 with pcf8574 i2c expander
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * (c) by Alexander Tuschen <atuschen@web.de>
 * (c) by Mirko Taschenberger <mirkiway at gmx.de>
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

/*
 * HD44780-Display über PCF8574 ansteuern.
 * Belegung Pollin Add-On-Board:
 *
 * Pin PCF8574		Pin am LCD
 *     4 (P0)	->	11 (DB4)
 *     5 (P1)	->	12 (DB5)
 *     6 (P2)	->	13 (DB6)
 *     7 (P3)	->	14 (DB7)
 *     9 (P4)	->	 4 (RS)
 *    10 (P5)	->	 5 (R/W) nicht benutzt !
 *    11 (P6)	->	 6 (EN)
 *    12 (P7)	->	15 (Beleuchtung)
 *
 * Die LCD-Beleuchtung an Pin 12 wird über einen 
 * PNP-Transistor geschaltet.
 * Beleuchtung an: Bit 7=0 
 * Beleuchtung aus: Bit 7=1
 * Die Address-Eingänge A0 bis A2 des PCF8574 liegen alle auf GND.
 * Die Basis-Addresse des Chips ist daher immer 0x20.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "hd44780.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pcf8574x.h"

/* port mapping lcd -> pcf8574x pin */
#define HD44780_PCF8574x_DB4			(0)
#define HD44780_PCF8574x_DB5			(1)
#define HD44780_PCF8574x_DB6			(2)
#define HD44780_PCF8574x_DB7			(3)
#define HD44780_PCF8574x_RS				(4)
#define HD44780_PCF8574x_WR				(5)
#define HD44780_PCF8574x_EN				(6)
#define HD44780_PCF8574x_BL				(7)

#define HD44780_PCF8574x_DBx_MASK		( _BV(HD44780_PCF8574x_DB4) | \
										  _BV(HD44780_PCF8574x_DB5) | \
										  _BV(HD44780_PCF8574x_DB6) | \
										  _BV(HD44780_PCF8574x_DB7) )

/* global variables */
uint8_t lcd_data;


uint8_t noinline clock_rw(uint8_t read,uint8_t en)
{
    uint8_t data = 0;

    if (read)
    {   /* Datenbyte an PCF senden, DBx high zum lesen, sollte vor EN high Flanke geschehen */
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data | HD44780_PCF8574x_DBx_MASK);
		/* EN setzen */
        lcd_data |= _BV(HD44780_PCF8574x_EN);
		/* Datenbyte an PCF senden, DBx high zum lesen, EN uebertragen */
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data | HD44780_PCF8574x_DBx_MASK);
		/* Datenbyte von PCF lesen, daten_nibble maskiert */
        data = i2c_pcf8574x_read(HD44780_PCF8574_ADR) & HD44780_PCF8574x_DBx_MASK;
    }
    else
    {   /* Datenbyte an PCF senden, muss vor EN high Flanke geschehen */
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);
		/* EN setzen */
        lcd_data |= _BV(HD44780_PCF8574x_EN);
		/* Datenbyte an PCF senden, EN status uebertragen */
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);
    }

	/* EN loeschen */
    lcd_data &= ~(_BV(HD44780_PCF8574x_EN));
	/* Datenbyte erneut senden, EN status uebertragen */
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);

    return data;
}

void noinline output_nibble(uint8_t rs, uint8_t nibble,uint8_t en)
{
    lcd_data = nibble;

	/* Wenn rs==1, dann RS setzen */
    if (rs)
        lcd_data |= _BV(HD44780_PCF8574x_RS);

	/* WR loeschen = schreiben */
    lcd_data &= ~(_BV(HD44780_PCF8574x_WR));

	/* backlight status falls vorhanden uebernehmen */
	if(back_light)
        lcd_data |= _BV(HD44780_PCF8574x_BL);
    else
        lcd_data &= ~(_BV(HD44780_PCF8574x_BL));

    /* toggle EN, daten uebertragen */
    clock_write(1); //FIXME: bei input_nibble steht hier 'en'
}

#ifdef HD44780_READBACK
uint8_t noinline input_nibble(uint8_t rs, uint8_t en)
{
    /* WR setzen = lesen */
    lcd_data |= _BV(HD44780_PCF8574x_WR);

    /* Wenn rs gesetzt, */
    if (rs)	/* dann RS setzen */
        lcd_data |= _BV(HD44780_PCF8574x_RS);
    else /* sonst RS loeschen */
        lcd_data &= ~(_BV(HD44780_PCF8574x_RS));

	/* toggle EN, daten abholen */
    uint8_t data = clock_read(en);

	/* WR loeschen = schreiben */
    lcd_data &= ~(_BV(HD44780_PCF8574x_WR));

    return data;
}
#endif

void hd44780_backlight(uint8_t state)
{
    back_light = state;

	/* backlight status falls vorhanden uebernehmen */
	if(back_light)
        lcd_data |= _BV(HD44780_PCF8574x_BL);
    else
        lcd_data &= ~(_BV(HD44780_PCF8574x_BL));
	/* backlight status direkt uebertragen */
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);
}

void noinline hd44780_hw_init(void)
{
	/* alle ausgaenge auf 0 setzen */
    lcd_data = 0;
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data );
}

