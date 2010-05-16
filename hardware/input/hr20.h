/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef _INPUT_HR20_H
#define _INPUT_HR20_H

struct hr20_input
{
  unsigned wheel_left:1;
  unsigned wheel_right:1;
  unsigned button_auto:1;
  unsigned button_prog:1;
  unsigned button_temp:1;
};

extern struct hr20_input hr20_input;

void hr20_input_init (void);

#define HR20_ACCESS(field) \
  (hr20_input.field ? hr20_input.field -- : 0)

#define HR20_INPUT_WHEEL_LEFT()  HR20_ACCESS(wheel_left)
#define HR20_INPUT_WHEEL_RIGHT() HR20_ACCESS(wheel_right)
#define HR20_INPUT_BUTTON_AUTO() HR20_ACCESS(button_auto)
#define HR20_INPUT_BUTTON_PROG() HR20_ACCESS(button_prog)
#define HR20_INPUT_BUTTON_TEMP() HR20_ACCESS(button_temp)

#endif /* _INPUT_HR20_H */
