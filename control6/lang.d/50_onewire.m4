dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl  Copyright (c) 2011 by Maximilian Güntner
dnl  Copyright (c) 2011-2012 by Erik Kunze <ethersex@erik-kunze.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

define(`ONEWIRE_USED', `ifdef(`onewire_used', `', `dnl
define(`old_divert', divnum)dnl
define(`onewire_used')dnl
divert(globals_divert)`
#ifndef ONEWIRE_SUPPORT
#error Please define onewire support
#endif

#include <util/atomic.h>
#include "hardware/onewire/onewire.h"

#ifdef ONEWIRE_POLLING_SUPPORT
static int16_t
ow_read_temp(ow_rom_code_t *rom)
{
  /*Search the sensor...*/
  for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
  {
    /*Maybe check here whether the device is a temperature sensor*/
    if(ow_sensors[i].ow_rom_code.raw == rom->raw)
    {
      /*Found it*/
      ow_temp_t temp = ow_sensors[i].temp;
      return (temp.twodigits ? temp.val / 10 : temp.val);
    }
  }
  /*Sensor is not in list*/
  return 0x7FFF;  /* error */

}
static int16_t
ow_temp (ow_rom_code_t *rom)
{
  return ow_read_temp(rom);
}
#else /*ONEWIRE_POLLING_SUPPORT is not defined*/
static int16_t
ow_read_temp(ow_rom_code_t *rom)
{
  int16_t retval = 0x7FFF;  /* error */
  ow_temp_scratchpad_t sp;
  int8_t ret;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ret = ow_temp_read_scratchpad(rom, &sp);
  }
  if (ret == 1)
  {
    ow_temp_t temp = ow_temp_normalize(rom, &sp);
    retval = (temp.twodigits ? temp.val / 10 : temp.val);
  }

  return retval;
}

static int16_t
ow_temp(ow_rom_code_t *rom)
{
  int16_t retval = 0x7FFF;  /* error */
  int8_t ret;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ret = ow_temp_start_convert_wait(rom);
  }
  if (ret == 1)
  {
    retval = ow_read_temp(rom);
  }

  return retval;
}
#endif /*ONEWIRE_POLLING_SUPPORT*/

#ifdef ONEWIRE_NAMING_SUPPORT
static int16_t
ow_temp_by_name(const char* name)
{
  int16_t retval = 0x7FFF;  /* error */

  ow_sensor_t *sensor = ow_find_sensor_name(name);
  if (sensor != NULL)
    retval = ow_temp(&sensor->ow_rom_code);

  return retval;
}
#endif /*ONEWIRE_NAMING_SUPPORT*/
'
divert(old_divert)')')

define(`ONEWIRE_GET', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef ONEWIRE_$1
ow_rom_code_t ow_$1 = {{ .bytewise = {
  `0x'substr($1,0,2), dnl
  `0x'substr($1,2,2), dnl
  `0x'substr($1,4,2), dnl
  `0x'substr($1,6,2), dnl
  `0x'substr($1,8,2), dnl
  `0x'substr($1,10,2), dnl
  `0x'substr($1,12,2), dnl
  `0x'substr($1,14,2), dnl
}}};
#define ONEWIRE_$1
#endif
divert(old_divert)ow_temp(&ow_$1)')

define(`ONEWIRE_GET_BY_NAME', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef ONEWIRE_$1
const char* ow_$1_name = "$1";
#define ONEWIRE_$1
#endif
divert(old_divert)ow_temp_by_name(ow_$1_name)')

define(`ONEWIRE_READ', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef ONEWIRE_$1
ow_rom_code_t ow_$1 = {{ .bytewise = {
  `0x'substr($1,0,2), dnl
  `0x'substr($1,2,2), dnl
  `0x'substr($1,4,2), dnl
  `0x'substr($1,6,2), dnl
  `0x'substr($1,8,2), dnl
  `0x'substr($1,10,2), dnl
  `0x'substr($1,12,2), dnl
  `0x'substr($1,14,2), dnl
}}};
#define ONEWIRE_$1
#endif
divert(old_divert)ow_read_temp(&ow_$1)')

define(`ONEWIRE_CONVERT', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
divert(old_divert)
#ifndef ONEWIRE_POLLING_SUPPORT
ow_temp_start_convert(NULL,0);
#endif // dirty workaround for -> ')
