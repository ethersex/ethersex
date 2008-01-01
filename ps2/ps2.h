/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */

#ifndef _ZBUS_H
#define _ZBUS_H
#include <stdint.h>

#define KEY_F1 0x05
#define KEY_F2 0x06
#define KEY_F3 0x04
#define KEY_F4 0x0c
#define KEY_F5 0x03
#define KEY_F6 0x0b
#define KEY_F7 0x83
#define KEY_F8 0x0a
#define KEY_F9 0x01
#define KEY_F10 0x09
#define KEY_F11 0x78
#define KEY_F12 0x07

#define KEY_ENTER 0x5a
#define KEY_BACKSPACE 0x66
#define KEY_ESC 0x76
#define KEY_TAB 0x0d

/* Are the same keycode as on the number block */
#define KEY_DEL 0x71
#define KEY_INSERT 0x70
#define KEY_HOME 0x6c
#define KEY_END 0x69
#define KEY_PAGE_UP 0x7d
#define KEY_PAGE_DOWN 0x7a

struct key_press {
  uint8_t keycode;

  /* modifiers */
  uint8_t ctrl:1;
  uint8_t alt:1;
  uint8_t lin:1;
  uint8_t num:1;
  uint8_t shift:1;
};

void ps2_init(void);
void ps2_periodic(void);

#endif /* _ZBUS_H */
