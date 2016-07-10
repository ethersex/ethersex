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
#include <util/atomic.h>

/*
* lome6 one wire sensor stuff
* tiny get temperature function (taken from control6)
*/
#ifdef LOME6_ONEWIRE_SUPPORT
int16_t
lome6_get_temperature(ow_rom_code_t * rom)
{
  int16_t retval = 0x7FFF;      /* error */

  ow_sensor_t * sensor = ow_find_sensor(rom);
  if (sensor != NULL)
  {
    ow_temp_t temp = sensor->temp;
    retval = (temp.twodigits ? temp.val / 10 : temp.val);
  }

  return retval;
}
#endif



/*
* lome6 startup function
* get the sensor id's from the config and convert them to ow_rom_code_t and read the scratchpads
* start a first one wire temperature convert
*/
void
lome6_startup(void)
{
  iLCDPage = 0;
  iTemperatureCPU = 0;
  iTemperatureSB = 0;
  iCountdownTimer = 0;
  iUptime = 0;
  iPOD = 0;

#ifdef LOME6_ONEWIRE_SUPPORT
  iTemperatureAIR = 0;
  iTemperaturePSU = 0;
  iTemperatureRAM = 0;
#endif

#ifdef LOME6_LCD_SUPPORT
  WINDOW *ttyWindow = NULL;
#endif

#ifdef LOME6_ONEWIRE_SUPPORT
  if (parse_ow_rom(CONF_SENSOR_PSU, &romcodePSU) == -1)
  {
    LOME6DEBUG("cannot parse ow rom code for psu sensor\n");
  }

  if (parse_ow_rom(CONF_SENSOR_AIR, &romcodeAIR) == -1)
  {
    LOME6DEBUG("cannot parse ow rom code for air sensor\n");
  }

  if (parse_ow_rom(CONF_SENSOR_RAM, &romcodeRAM) == -1)
  {
    LOME6DEBUG("cannot parse ow rom code for ram sensor\n");
  }
#endif

#ifdef LOME6_LCD_SUPPORT
  ttyWindow = subwin(NULL, CONF_LOME6_LCD_HEIGHT, CONF_LOME6_LCD_WIDTH, 0, 0);
#endif

  // only set iPOD if server not running
  // (disable power on delay if server is running already)
  if (PIN_HIGH(POWER_STATE))
    iPOD = CONF_LOME6_POD;
}



/*
* lome6 periodical timer function for display and one wire convert command
*
* if onewire is supported start onewire temperature convert
* if lcd is supported display various information
*/
void
lome6_timer(void)
{
#ifdef LOME6_ONEWIRE_SUPPORT
  // read 1w temperatures
  iTemperaturePSU = lome6_get_temperature(&romcodePSU);
  iTemperatureAIR = lome6_get_temperature(&romcodeAIR);
  iTemperatureRAM = lome6_get_temperature(&romcodeRAM);
#endif // LOME6_ONEWIRE_SUPPORT

#ifdef LOME6_LCD_SUPPORT
  wclear(ttyWindow);

  if (iLCDPage == 0)
  {
    // display uptime and date+time
    uint32_t working_hours = clock_get_uptime() / 60;

    clock_datetime_t datetime;
    clock_current_localtime(&datetime);

    wprintw_P(ttyWindow, PSTR("%02d:%02d %02d.%02d.%04d"), datetime.hour,
              datetime.min, datetime.day, datetime.month,
              (datetime.year + 1900));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("Uptime: %02lu:%02d"), working_hours / 60,
              working_hours % 60);
    wclrtoeol(ttyWindow);

#ifndef LOME6_ONEWIRE_SUPPORT
    if (!PIN_HIGH(POWER_STATE))
      iLCDPage = 4;
    else
      iLCDPage = 0;
#else
    iLCDPage++;
#endif

#ifdef LOME6_ONEWIRE_SUPPORT
  }
  else if (iLCDPage == 1)
  {
    // display onewire temperature sensor data
    wprintw_P(ttyWindow, PSTR("Temperature"));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("AIR: %02d.%d"), iTemperatureAIR / 10,
              iTemperatureAIR % 10);
    wclrtoeol(ttyWindow);

    iLCDPage++;
  }
  else if (iLCDPage == 2)
  {
    // display onewire temperature sensor data
    wprintw_P(ttyWindow, PSTR("Temperature:"));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("RAM: %02d.%d"), iTemperatureRAM / 10,
              iTemperatureRAM % 10);
    wclrtoeol(ttyWindow);

    iLCDPage++;
  }
  else if (iLCDPage == 3)
  {
    // display onewire temperature sensor data
    wprintw_P(ttyWindow, PSTR("Temperature"));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("PSU: %02d.%d"), iTemperaturePSU / 10,
              iTemperaturePSU % 10);
    wclrtoeol(ttyWindow);

    iLCDPage++;
#endif //LOME6_ONEWIRE_SUPPORT
  }
  else if (iLCDPage == 4)
  {
    // display temperature data
    wprintw_P(ttyWindow, PSTR("Temperature"));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("CPU: %02d.%d"), iTemperatureCPU / 10,
              iTemperatureCPU % 10);
    wclrtoeol(ttyWindow);
    iLCDPage++;
  }
  else if (iLCDPage == 5)
  {
    // display temperature data
    wprintw_P(ttyWindow, PSTR("Temperature"));
    wclrtoeol(ttyWindow);
    wmove(ttyWindow, 1, 0);
    wprintw_P(ttyWindow, PSTR("SB: %02d.%d"), iTemperatureSB / 10,
              iTemperatureSB % 10);
    wclrtoeol(ttyWindow);
    iLCDPage = 0;
  }
#endif // LOME6_LCD_SUPPORT
}


/*
* lome6 periodical countdown timer function
* counts down time in ~seconds since power button long is pressed (needed to prevent watchdog action)
* manages power on delay
*/
void
lome6_timersec(void)
{
  if (iCountdownTimer > 0)
  {
    iCountdownTimer--;
    if (iCountdownTimer <= 0)
      PIN_CLEAR(RELAIS_POWER);
  }

  // test power on delay counter
  // if server already on, dont do anything
  if (iPOD > 0 && PIN_HIGH(POWER_STATE))
  {
    iPOD--;
    if (iPOD <= 0)
    {
      PIN_SET(RELAIS_POWER);
      _delay_ms(CONF_TIME2PRESS_POWER);
      PIN_CLEAR(RELAIS_POWER);
    }
  }
}

/*
  -- Ethersex META --
  block(lome6)
  header(services/lome6/lome6.h)
  startup(lome6_startup)
  timer(300, lome6_timer())
  timer(50, lome6_timersec())
*/
