/*
*
* Copyright (c) 2012 by Daniel Lindner <daniel.lindner@gmx.de>
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

#ifndef _BSBPORT_HELPER_H
#define _BSBPORT_HELPER_H

uint16_t bsbport_crc(const uint8_t * const buffer, const uint8_t length);
int16_t bsbport_ConvertToInt16(const uint8_t * const msg);

#endif /* _BSBPORT_HELPER_H */
