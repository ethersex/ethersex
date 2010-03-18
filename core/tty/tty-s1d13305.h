/*
 * Copyright (c) 2009 by Daniel Walter <fordprfkt@googlemail.com>
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

#ifndef TTY_S1D13305_H
#define TTY_S1D13305_H

#include "hardware/lcd/s1d13305/s1d13305.h"

#define tty_s1d13305_clear()	 lcd_clear (LCD_LAYER1)
#define tty_s1d13305_goto(y,x)	 {}
#define tty_s1d13305_put(x,y,ch) lcd_writeTextPos(x, y, ch, 1)

#endif	/* TTY_S1D13305_H */
