/*
 * Copyright (c) 2015 Meinhard Ritscher <unreachable@gmx.net>
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

#include "fat_config.h"
#include "services/clock/clock.h"

/*
 * see fat_config.h for documentation
 */
void
get_datetime(uint16_t * year, uint8_t * month, uint8_t * day, uint8_t * hour,
             uint8_t * min, uint8_t * sec)
{
  clock_datetime_t date;
  clock_current_localtime(&date);

  *day = date.day;
  *month = date.month;
  *year = date.year + 1900;
  *hour = date.hour;
  *min = date.min;
  *sec = date.sec;
}
