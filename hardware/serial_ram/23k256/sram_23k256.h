/*
*
* Copyright (c) 2012 by Daniel Walter <fordprfkt@googlemail.com>
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

#ifndef HAVE_SRAM_23K256_H
#define HAVE_SRAM_23K256_H

#define SRAM23K256_SIZE 32768  /* Size of the RAM in bytes */

int16_t sram23k256_init(void);
void sram23k256_read(uint16_t address_ui16, uint8_t dataPtr_pui8[], uint8_t len_ui8);
void sram23k256_write(uint16_t address_ui16, uint8_t dataPtr_pui8[], uint8_t len_ui8);

#include "config.h"
#ifdef DEBUG_SER_RAM_23K256
# include "core/debug.h"
# define SERRAMDEBUG(a...)  debug_printf("serial ram: " a)
#else
# define SERRAMDEBUG(a...)
#endif

#endif  /* HAVE_SRAM_23K256_H */
