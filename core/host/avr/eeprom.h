/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef HOST_AVR_EEPROM_H
#define HOST_AVR_EEPROM_H

#include <stdint.h>

#define EEPROM_SIZE 2048
extern uint8_t eeprom_data[EEPROM_SIZE];

#define eeprom_write_byte(ptr,val)	(eeprom_data[(int)ptr] = val)

#define eeprom_read_byte(ptr)		(eeprom_data[(int)ptr])
#define eeprom_read_block(dst,src,n)	memmove(dst,&eeprom_data[(int)src],n)

void eeprom_host_init (void);
void eeprom_host_exit (void);

#endif	/* HOST_AVR_EEPROM_H */
