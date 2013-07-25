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

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define MOD_NONE            0
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

#define KEY_errorRollOver   1
#define KEY_POSTfail        2
#define KEY_errorUndefined  3
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

#define KEY_ENTER   40
#define KEY_ESC     41
#define KEY_BCKSPACE 42 // backspace
#define KEY_TAB     43
#define KEY_SPACE   44 // space
#define KEY_MINUS   45 // - (and _)
#define KEY_EQUALS  46 // =
#define KEY_LBR     47 // [
#define KEY_RBR     48 // ]  -- 0x30
#define KEY_BCKSLASH 49 // \ (and |)
#define KEY_HASH    50 // Non-US # and ~
#define KEY_SMCOL   51 // ; (and :)
#define KEY_PING    52 // ' and "
#define KEY_GRAVE   53 // Grave accent and tilde
#define KEY_COMMA   54 // , (and <)
#define KEY_DOT     55 // . (and >)
#define KEY_SLASH   56 // / (and ?)
#define KEY_CAPSLOCK  57 // capslock
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

#define KEY_PRTSCR  70
#define KEY_SCRLCK  71
#define KEY_BREAK   72
#define KEY_INS     73
#define KEY_HOME    74
#define KEY_PGUP    75
#define KEY_DEL     76
#define KEY_END     77
#define KEY_PGDN    78
#define KEY_RARR    79 
#define KEY_LARR    80     // 0x50
#define KEY_DARR    81
#define KEY_UARR    82
#define KEY_NUMLOCK 83
#define KEY_KPSLASH 84
#define KEY_KPAST   85
#define KEY_KPMINUS 86
#define KEY_KPPLUS  87
#define KEY_KPENTER 88
#define KEY_KP1     89
#define KEY_KP2     90
#define KEY_KP3     91
#define KEY_KP4     92
#define KEY_KP5     93
#define KEY_KP6     94
#define KEY_KP7     95
#define KEY_KP8     96      // 0x60
#define KEY_KP9     97
#define KEY_KP0     98
#define KEY_KPCOMMA 99
#define KEY_EURO2  100



#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef schar
#define schar   signed char
#endif

void usb_keyboard_periodic(void);
/* Initialize USB keyboard. */
void usb_keyboard_periodic_call(void);

void usb_keyboard_init(void);

struct hid_keyboard_map_t {
  uchar character;
  uchar reportBuffer[2];    /* buffer for HID reports */
};

uint16_t hid_usbFunctionSetup(uchar data[8]);

#include "config.h"
#ifdef DEBUG_USB_HID_KEYBOARD
# include "core/debug.h"
# define USBKEYBOARDDEBUG(a...)  debug_printf("usb keyb: " a)
#else
# define USBKEYBOARDDEBUG(a...)
#endif


#endif /* __USB_HID_KEYBOARD_H */
