/*
*
* Copyright (c) 2011 by Gerd v. Egidy <gerd@egidy.de>
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

#ifndef DALI_H
#define DALI_H

#ifdef DALI_SUPPORT

// DALI uses 1200 baud, this waits for half of one bit time
#define DALI_HALF_BIT_USEC 416
#define DALI_HALF_BIT_WAIT _delay_us(DALI_HALF_BIT_USEC)

#define DALI_READ_TIMEOUT -1
#define DALI_READ_ERROR -2
#define DALI_READ_OK 1

void dali_send(uint16_t *frame);
int8_t dali_read(uint8_t *frame);

#endif /* SHT_SUPPORT */

#endif /* SHT_H */
