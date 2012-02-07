/*
 * Copyright (c) 2009 by Michael Stapelberg <michael+es@stapelberg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef _SRAM_H
#define _SRAM_H

#define SRAM_START_ADDRESS (uint8_t*)0x1100
#define SRAM_END_ADDRESS (uint8_t*)0xFFFF

/* debugging support */
#if 1
# define SRAM_DEBUG(a...) debug_printf("sram: " a)
#else
# define SRAM_DEBUG(a...) do { } while(0)
#endif

void sram_init(void);

#endif
