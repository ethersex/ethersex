/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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

#ifndef _USB_HID_KEYBOARD_H
#define _USB_HID_KEYBOARD_H

//#define USB_CFG_HAVE_INTRIN_ENDPOINT    1
//#define USB_CFG_INTERFACE_CLASS     0x03    /* HID class */
//#define USB_CFG_INTR_POLL_INTERVAL      10

  /* total length of report descriptor */
//#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH sizeof(usbHidReportDescriptor)
//#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH 35

#define USB_CFG_INTERFACE_CLASS     0x03

#include "protocols/usb/usbdrv/usbdrv.h"



#define NUM_KEYS    17

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

#define KEY_A       4
#define KEY_B       5
#define KEY_C       6
#define KEY_D       7
#define KEY_E       8
#define KEY_F       9
#define KEY_G       10
#define KEY_H       11
#define KEY_I       12
#define KEY_J       13
#define KEY_K       14
#define KEY_L       15
#define KEY_M       16
#define KEY_N       17
#define KEY_O       18
#define KEY_P       19
#define KEY_Q       20
#define KEY_R       21
#define KEY_S       22
#define KEY_T       23
#define KEY_U       24
#define KEY_V       25
#define KEY_W       26
#define KEY_X       27
#define KEY_Y       28
#define KEY_Z       29
#define KEY_1       30
#define KEY_2       31
#define KEY_3       32
#define KEY_4       33
#define KEY_5       34
#define KEY_6       35
#define KEY_7       36
#define KEY_8       37
#define KEY_9       38
#define KEY_0       39

#define KEY_F1      58
#define KEY_F2      59
#define KEY_F3      60
#define KEY_F4      61
#define KEY_F5      62
#define KEY_F6      63
#define KEY_F7      64
#define KEY_F8      65
#define KEY_F9      66
#define KEY_F10     67
#define KEY_F11     68
#define KEY_F12     69


extern uchar reportBuffer[2];    /* buffer for HID reports */
extern uchar idleRate;           /* in 4 ms units */
extern uint8_t key;

void buildReport(uchar key);

/* Initialize USB keyboard. */
void usb_keyboard_init (void);

void usb_keyboard_periodic (void);

void usb_keyboard_periodic_call (void);

#include "config.h"
#ifdef DEBUG_USB_HID_KEYBOARD
# include "core/debug.h"
# define USBKEYBOARDDEBUG(a...)  debug_printf("usb keyb: " a)
#else
# define USBKEYBOARDDEBUG(a...)
#endif


#endif /* __USB_HID_KEYBOARD_H */
