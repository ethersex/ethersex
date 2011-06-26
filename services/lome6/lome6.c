/*
*
* Copyright (c) 2011 by warhog <warhog@gmx.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
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

#include "lome6.h"

/*
* lome6 lcd stuff
*/
#ifdef LOME6_LCD_SUPPORT
void lome6_lcdString(char *string) {

	for (uint8_t i = 0; i < strlen(string); i++)
		putc(string[i], lcd);

}

void lome6_lcdClear(void) {

	hd44780_clear();
	hd44780_goto(0, 0);

}

void lome6_output_lcd(char *line1, char *line2) {

	lome6_lcdString(line1);
	lome6_lcdGoto(0, 1);
	lome6_lcdString(line2);

}


void lome6_lcd_temperature(char *type, int temperature, int decimal) {

	lome6_lcdGoto(0, 0);
	lome6_lcdString("Temperature");

	char string[17];

	snprintf_P(string, sizeof(string), type, temperature, decimal);
	lome6_lcdGoto(0, 1);
	lome6_lcdString(string);

}
#endif


/*
* lome6 one wire sensor stuff
* tiny get temperature function
*/
#ifdef LOME6_ONEWIRE_SUPPORT
int16_t lome6_get_temperature(struct ow_rom_code_t *rom) {

	void *addr = rom->bytewise;

	int16_t retval = 0x7FFF;  /* error */

	struct ow_temp_scratchpad_t sp;
	if (ow_temp_read_scratchpad(addr, &sp) != 1)
		goto out;  // scratchpad read failed

	uint16_t temp = ow_temp_normalize(addr, &sp);
	retval = HI8(temp) * 10 + HI8(((temp & 0x00ff) * 10) + 0x80);

	out:
	return retval;

}
#endif



/*
* lome6 startup function
* get the sensor id's from the config and convert them to struct ow_rom_code_t and read the scratchpads
* start a first one wire temperature convert
*/
void lome6_startup(void) {

#ifdef LOME6_ONEWIRE_SUPPORT
	parse_ow_rom(CONF_SENSOR_PSU, &romcodePSU);
	parse_ow_rom(CONF_SENSOR_AIR, &romcodeAIR);
	parse_ow_rom(CONF_SENSOR_RAM, &romcodeRAM);

	ow_temp_start_convert_nowait(NULL);
#endif

}



/*
* lome6 periodical timer function for display and one wire convert command
*
* if onewire is supported start onewire temperature convert
* if lcd is supported display various information
*/
void lome6_timer(void) {

#ifdef LOME6_ONEWIRE_SUPPORT
	// read 1w temperatures
	iTemperaturePSU = lome6_get_temperature(&romcodePSU);
	iTemperatureAIR = lome6_get_temperature(&romcodeAIR);
	iTemperatureRAM = lome6_get_temperature(&romcodeRAM);

	// start a new convert for the next round
	ow_temp_start_convert_nowait(NULL);
#endif

#ifdef LOME6_LCD_SUPPORT
	lome6_lcdClear();
	char lcd_line1[17];
	char lcd_line2[17];
	memset(lcd_line1, ' ', sizeof(lcd_line1));
	memset(lcd_line2, ' ', sizeof(lcd_line2));

	if (iLCDPage == 0) {

		// display uptime and date+time
		uint32_t working_hours = (clock_get_time() - clock_get_startup()) / 60;

		struct clock_datetime_t datetime;
		clock_current_localtime(&datetime);

		snprintf_P(lcd_line1, sizeof(lcd_line1), PSTR("%02d:%02d %02d/%02d/%04d"), datetime.hour, datetime.min, datetime.day, datetime.month, (datetime.year + 1900));
		snprintf_P(lcd_line2, sizeof(lcd_line2), PSTR("Uptime: %02lu:%02d"), working_hours / 60, working_hours % 60);

		lome6_output_lcd(lcd_line1, lcd_line2);

#ifndef LOME6_ONEWIRE_SUPPORT
		if (PIN_HIGH(POWER_STATE))
			iLCDPage = 4;
		else
			iLCDPage = 0;
#else
		iLCDPage++;
#endif

#ifdef LOME6_ONEWIRE_SUPPORT
	} else if (iLCDPage == 1) {

		// display onewire temperature sensor data
		lome6_lcd_temperature(PSTR("AIR: %d.%d"), iTemperatureAIR / 10, iTemperatureAIR % 10);
		iLCDPage++;

	} else if (iLCDPage == 2) {

		// display onewire temperature sensor data
		lome6_lcd_temperature(PSTR("RAM: %d.%d"), iTemperatureRAM / 10, iTemperatureRAM % 10);
		iLCDPage++;

	} else if (iLCDPage == 3) {

		// display onewire temperature sensor data
		lome6_lcd_temperature(PSTR("PSU: %d.%d"), iTemperaturePSU / 10, iTemperaturePSU % 10);
		iLCDPage++;

#endif //LOME6_ONEWIRE_SUPPORT
	} else if (iLCDPage == 4) {
	
		lome6_lcd_temperature(PSTR("CPU: %d.%d"), iTemperatureCPU / 10, iTemperatureCPU % 10);
		iLCDPage++;

	} else if (iLCDPage == 5) {

		lome6_lcd_temperature(PSTR("SB: %d.%d"), iTemperatureSB / 10, iTemperatureSB % 10);
		iLCDPage = 0;

	}
#endif // LOME6_LCD_SUPPORT

}


/*
* lome6 periodical countdown timer function
*
* counts down time in ~seconds since power button long is pressed (needed to prevent watchdog action)
*/
void lome6_timersec(void) {

	if (iCountdownTimer > 0) {

		iCountdownTimer--;

		if (iCountdownTimer == 0) {

			PIN_CLEAR(RELAIS_POWER);

		}

	}

}

/*
  -- Ethersex META --
  block(lome6)
  startup(lome6_startup)
  timer(300, lome6_timer())
  timer(50, lome6_timersec())
*/
