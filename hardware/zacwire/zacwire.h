/*
*
* Copyright (c) 2009 by Gerd v. Egidy <gerd@egidy.de>
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

#ifndef ZACWIRE_H
#define ZACWIRE_H

#ifdef ZACWIRE_SUPPORT

#define ZACWIRE_OK 0
#define ZACWIRE_ERR_TIMEOUT -1
#define ZACWIRE_ERR_PROTOCOL -2
#define ZACWIRE_ERR_PARITY -3

int8_t zacwire_get(uint16_t *raw_temp, uint8_t *pin_register, uint8_t bitno);
int16_t convert_tsic306(uint16_t raw_temp);
int16_t convert_tsic506(uint16_t raw_temp);

#endif /* ZACWIRE_SUPPORT */

#endif /* ZACWIRE_H */
