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
 * HD44780-Display Ã¼ber PCF8574 ansteuern.
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
 * Die LCD-Beleuchtung an Pin 12 wird Ã¼ber einen 
 * PNP-Transistor geschaltet.
 * Beleuchtung an: Bit 7=0 
 * Beleuchtung aus: Bit 7=1
 * Die Address-EingÃ¤nge A0 bis A2 des PCF8574 liegen alle auf GND.
 * Die Basis-Addresse des Chips ist daher immer 0x20.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "hd44780.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_pcf8574x.h"

/* global variables */
extern FILE *lcd;
extern uint8_t back_light;
uint8_t lcd_data;


uint8_t noinline clock_rw(uint8_t read,uint8_t en)
{
    uint8_t data = 0;

    if (read)
    {
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data | 0x0f);	//Datenbyte an PCF senden, DBx high zum lesen, 
		                                                        // sollte vor EN high Flanke geschehen
        lcd_data |= _BV(6);					//Bit 6 (EN) setzen
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data | 0x0f);	//Datenbyte an PCF senden, DBx high zum lesen, EN uebertragen
        data = i2c_pcf8574x_read(HD44780_PCF8574_ADR) & 0x0f;	//Datenbyte von PCF lesen, unteres nibble
    }
    else
    {
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);	//Datenbyte an PCF senden, muss vor EN high Flanke geschehen
        lcd_data |= _BV(6);					//Bit 6 (EN) setzen
        i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);	//Datenbyte an PCF senden, EN status uebertragen
    }

    lcd_data &= ~(_BV(6));					//Bit 6 (EN) löschen
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);		//Datenbyte erneut senden, EN status uebertragen

    return data;
}

void noinline output_nibble(uint8_t rs, uint8_t nibble,uint8_t en)
{
    lcd_data = nibble;

    if (rs)				//Falls rs==1
        lcd_data |= _BV(4);		//Bit 4 (RS) setzen

    lcd_data &= ~(_BV(5));		//Bit 5 (WR) löschen = schreiben

    if(back_light)			//backlight support
        lcd_data |= _BV(7);
    else
        lcd_data &= ~(_BV(7));

    // toggle EN
    clock_write(1);
}

#ifdef HD44780_READBACK
uint8_t noinline input_nibble(uint8_t rs, uint8_t en)
{
    // set write bit
    lcd_data |= _BV(5);			//Bit 5 (WR) setzen = lesen

    // set rs, if given
    if (rs)				//Falls rs==1
        lcd_data |= _BV(4);		//Bit 4 (RS) setzen
    else
        lcd_data &= ~(_BV(4));		//Bit 4 (RS) löschen

    uint8_t data = clock_read(en);

    // delete RW bit
    lcd_data &= ~(_BV(5));		//Bit 5 (WR) löschen = schreiben

    return data;
}
#endif

void hd44780_backlight(uint8_t state)
{
    back_light = state;

    if(back_light)			//backlight support
        lcd_data |= _BV(7);
    else
        lcd_data &= ~(_BV(7));
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, lcd_data);	// Byte senden
}

void noinline hd44780_hw_init(void)
{
    lcd_data=0;
    i2c_pcf8574x_set(HD44780_PCF8574_ADR, 0 );	// Alle Ausgänge auf 0 setzen
}
