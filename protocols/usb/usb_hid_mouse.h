/*
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
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
 */

#ifndef _USB_HID_MOUSE_H
#define _USB_HID_MOUSE_H

#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef schar
#define schar   signed char
#endif

void usb_mouse_periodic(void);
/* Initialize USB mouse. */
void usb_mouse_periodic_call(void);

uint16_t hid_usbFunctionSetup(uchar data[8]);


struct hid_mouse_map_t {
  uchar button;
  char deltax;
  char deltay;
};

#include "config.h"
#ifdef DEBUG_USB_HID_MOUSE
# include "core/debug.h"
# define USBMOUSEDEBUG(a...)  debug_printf("usb mouse: " a)
#else
# define USBMOUSEDEBUG(a...)
#endif

#endif /* __USB_HID_MOUSE_H */
