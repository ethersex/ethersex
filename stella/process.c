/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#include <stdint.h>
#include "stella.h"


void
stella_process (unsigned char *buf, uint8_t len)
{
  int re_sort = 0;

  for (; len >= 2; buf += 2, len -= 2)
    switch (*buf)
      {
      case STELLA_SET_RED:
	stella_fade[STELLA_RED] = buf[1];
	stella_color[STELLA_RED] = buf[1];
	re_sort = 1;
	break;

      case STELLA_SET_GREEN:
	stella_fade[STELLA_GREEN] = buf[1];
	stella_color[STELLA_GREEN] = buf[1];
	re_sort = 1;
	break;

      case STELLA_SET_BLUE:
	stella_fade[STELLA_BLUE] = buf[1];
	stella_color[STELLA_BLUE] = buf[1];
	re_sort = 1;
	break;

      case STELLA_FADE_RED:
	stella_fade[STELLA_RED] = buf[1];
	break;

      case STELLA_FADE_GREEN:
	stella_fade[STELLA_GREEN] = buf[1];
	break;

      case STELLA_FADE_BLUE:
	stella_fade[STELLA_BLUE] = buf[1];
	break;
      }

  if (re_sort)
    stella_sort (stella_color);
}
