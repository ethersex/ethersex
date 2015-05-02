/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _BIT_MACROS_H
#define _BIT_MACROS_H

#include <stdlib.h>

#define HI8(x)  ((uint8_t)((x) >> 8))
#define LO8(x)  ((uint8_t)(x))

#define HI4(x)  ((uint8_t)((x) >> 4))
#define LO4(x)  ((uint8_t)((x) & 0xF))

#define HTONL(x) ((uint32_t)(((x) & 0xFF000000) >> 24) \
                | (uint32_t)(((x) & 0x00FF0000) >> 8) \
                | (uint32_t)(((x) & 0x0000FF00) << 8) \
                | (uint32_t)(((x) & 0x000000FF) << 24))

#define NTOHL(x) HTONL(x)

#define XOR_LOG(a,b) ((1 && (a)) ^ ((b) && 1))

#endif /* _BIT_MACROS_H */
