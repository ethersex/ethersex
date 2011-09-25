dnl
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "hardware/onewire/onewire.h"
#include "core/bit-macros.h"

#ifdef ONEWIRE_POLLING_SUPPORT
int16_t ow_read_temp (struct ow_rom_code_t *rom)
{
	/*Search the sensor...*/
	for(uint8_t i=0;i<OW_SENSORS_COUNT;i++)
	{
		/*Maybe check here whether the device is a temperature sensor*/
		if(ow_sensors[i].ow_rom_code.raw == rom->raw)
		{
			/*Found it*/
			int16_t temp = ow_sensors[i].temp;
			return temp;
		}
	}
	/*Sensor is not in list*/
	return 0x7FFF;  /* error */

}
int16_t ow_temp (struct ow_rom_code_t *rom)
{
	return ow_read_temp(rom);
}
#else /*ONEWIRE_POLLING_SUPPORT is not defined*/
int16_t ow_read_temp (struct ow_rom_code_t *rom)
{
  int16_t retval = 0x7FFF;  /* error */

  /* disable interrupts */
  uint8_t sreg = SREG;
  cli();

  struct ow_temp_scratchpad_t sp;
  if (ow_temp_read_scratchpad(rom, &sp) != 1)
    goto out;  // scratchpad read failed

  uint16_t temp = ow_temp_normalize(rom, &sp);
  retval = ((int8_t) HI8(temp)) * 10 + HI8(((temp & 0x00ff) * 10) + 0x80);

 out:
  SREG = sreg;
  return retval;
}

int16_t ow_temp (struct ow_rom_code_t *rom)
{
  int16_t retval = 0x7FFF;  /* error */

  /* disable interrupts */
  uint8_t sreg = SREG;
  cli();

  if (ow_temp_start_convert_wait(rom) != 1)
    goto out;
  
 retval=ow_read_temp(rom);
 out:
  SREG = sreg;
  return retval;
}
#endif /*ONEWIRE_POLLING_SUPPORT*/
'
divert(old_divert)')')

define(`ONEWIRE_GET', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef ONEWIRE_$1
struct ow_rom_code_t ow_$1 = {{ .bytewise = {
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

define(`ONEWIRE_READ', `ONEWIRE_USED()dnl
define(`old_divert', divnum)dnl
divert(globals_divert)
#ifndef ONEWIRE_$1
struct ow_rom_code_t ow_$1 = {{ .bytewise = {
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
