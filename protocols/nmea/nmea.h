/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef NMEA_H
#define NMEA_H 1

#include <stdint.h>

/* 
   Example data:
   $GPGGA,174014.360,4923.1834,N,01011.2252,E,1,04

   01234567890123456789012345678901234567890123456
   0         1         2         3         4
 */

struct nmea_t
{
  unsigned locked	: 1;
  unsigned valid	: 1;
  unsigned ptr		: 6;

  uint8_t latitude[9];
  uint8_t latitude_dir;

  uint8_t longitude[10];
  uint8_t longitude_dir;

  uint8_t satellites;
};

extern struct nmea_t nmea_data;

void nmea_init(void);

#endif	/* NMEA_H */
