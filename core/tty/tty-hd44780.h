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

#ifndef TTY_HD44780_H
#define TTY_HD44780_H

#include "hardware/lcd/hd44780.h"

#define tty_ll_clear() hd44780_clear ();

extern uint8_t tty_hd44780_y;
extern uint8_t tty_hd44780_x;

inline static void
tty_ll_goto (uint8_t y, uint8_t x)
{
  if (y == tty_hd44780_y && x == tty_hd44780_x)
    return;			/* No change. */
  hd44780_goto (y, x);
}

#endif	/* TTY_HD44780_H */
