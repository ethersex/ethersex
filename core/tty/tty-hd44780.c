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

#include "core/tty/tty-hd44780.h"

uint8_t tty_hd44780_y;
uint8_t tty_hd44780_x;

void
tty_ll_put (uint8_t y, uint8_t x, uint8_t ch)
{
  if (tty_hd44780_y != y || tty_hd44780_x != x)
    hd44780_goto (y, x);
  putc (ch, lcd);

  tty_hd44780_x ++;		/* Wrapping handled by tty main layer. */
}
