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

uint8_t stella_color[] = { 0, 0, 0};
uint8_t stella_fade[] = { 0, 0, 0};

void
stella_timer (void)
{
  int re_sort = 0;

  for (int i = 0; i < PINS; i ++) 
    {
      if (stella_color[i] == stella_fade[i])
	continue;

      if (stella_color[i] > stella_fade[i])
	stella_color[i] --;
      else /* stella_color[i] < stella_fade[i] */
	stella_color[i] ++;

      re_sort = 1;
    }

  if (re_sort)
    stella_sort (stella_color);
}
