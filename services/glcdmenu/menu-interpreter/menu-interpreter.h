#ifndef MENU_INTERPRETER_H
	#define MENU_INTERPRETER_H

/* MenuInterpreter
  Version 1.4
  (c) 2009-2010 by Malte Marwedel
  www.marwedels.de/malte

  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "menu-interpreter-config.h"

//generate many printf messages:
//#define DEBUG

#ifdef USE16BITADDR
typedef unsigned short MENUADDR;
#else
typedef unsigned long MENUADDR;
#endif


#ifdef LARGESCREEN
typedef unsigned short SCREENPOS;
#else
typedef unsigned char SCREENPOS;
#endif

/* Some screens take up 700000 ticks for drawing, if this is too much for a
multi threaded program, define MENU_SCHEDULE with your thread switch function and
this will result in a thread switch after drawing of every object.
Make sure, no menu_ functions are entered multiple times, as their behaviour
may be undefined in this case.

example for Nut/OS:
#include <sys/thread.h>
#define MENU_SCHEDULE NutThreadYield();

 */

#ifndef MENU_SCHEDULE
#define MENU_SCHEDULE
#endif


/* If you want to use a touch screen or mice, you may want to enable this in
order to have a function, which accepts Screen positions, searches a proper object
on this location and runs the action.
NOTE: The List element is only partially supported by now.
*/
//#define MENU_MOUSE_SUPPORT

//Implement the five functions by your own:
extern unsigned char menu_byte_get(MENUADDR addr);
extern void menu_screen_set(SCREENPOS x, SCREENPOS y, unsigned char color);
extern void menu_screen_flush(void);
extern void menu_screen_clear(void);
extern unsigned char menu_action(unsigned short action);

//arrys for dynamic data
extern unsigned char * menu_strings[];
extern unsigned char menu_checkboxstate[];
extern unsigned char menu_radiobuttonstate[];
extern unsigned short menu_listindexstate[];
extern unsigned char * menu_gfxdata[];

//header identify bytes
#define MENU_INVALID 0
#define MENU_BOX 1
#define MENU_LABEL 2
#define MENU_BUTTON 3
#define MENU_GFX 4
#define MENU_LIST 5
#define MENU_CHECKBOX 6
#define MENU_RADIOBUTTON 7
#define MENU_SUBWINDOW 8
#define MENU_WINDOW 9
#define MENU_SHORTCUT 10

//options bit definitions
#define MENU_OPTIONS_FOCUSABLE 0
#define MENU_OPTIONS_RECTANGLE 1
#define MENU_OPTIONS_COMPRESSED 2
#define MENU_OPTIONS_STORAGE 3

//number of data bytes for each object (excluding header identify byte)
#ifdef USE16BITADDR

#define MENU_ADDR_BYTES 2

#ifdef LARGESCREEN

//16 Bit large screen
#define MENU_INVALID_DATA 0
#define MENU_BOX_DATA 12
#define MENU_LABEL_DATA 7
#define MENU_BUTTON_DATA 14
#define MENU_GFX_DATA 13
#define MENU_LIST_DATA 19
#define MENU_CHECKBOX_DATA 15
#define MENU_RADIOBUTTON_DATA 15
#define MENU_SUBWINDOW_DATA 9
#define MENU_WINDOW_DATA 3
#define MENU_SHORTCUT_DATA 5

#else
//16 Bit normal screen
#define MENU_INVALID_DATA 0
#define MENU_BOX_DATA 10
#define MENU_LABEL_DATA 6
#define MENU_BUTTON_DATA 12
#define MENU_GFX_DATA 11
#define MENU_LIST_DATA 17
#define MENU_CHECKBOX_DATA 13
#define MENU_RADIOBUTTON_DATA 13
#define MENU_SUBWINDOW_DATA 7
#define MENU_WINDOW_DATA 3
#define MENU_SHORTCUT_DATA 5

#endif

#else

#define MENU_ADDR_BYTES 3

#ifdef LARGESCREEN
//24 Bit large screen
#define MENU_INVALID_DATA 0
#define MENU_BOX_DATA 13
#define MENU_LABEL_DATA 9
#define MENU_BUTTON_DATA 16
#define MENU_GFX_DATA 15
#define MENU_LIST_DATA 21
#define MENU_CHECKBOX_DATA 17
#define MENU_RADIOBUTTON_DATA 17
#define MENU_SUBWINDOW_DATA 9
#define MENU_WINDOW_DATA 3
#define MENU_SHORTCUT_DATA 6

#else
//24 Bit normal screen
#define MENU_INVALID_DATA 0
#define MENU_BOX_DATA 11
#define MENU_LABEL_DATA 8
#define MENU_BUTTON_DATA 14
#define MENU_GFX_DATA 13
#define MENU_LIST_DATA 19
#define MENU_CHECKBOX_DATA 15
#define MENU_RADIOBUTTON_DATA 15
#define MENU_SUBWINDOW_DATA 7
#define MENU_WINDOW_DATA 3
#define MENU_SHORTCUT_DATA 6

#endif

#endif


/*Some magic constans needed to find the right place of the information
*/
#ifdef LARGESCREEN

//offset for all focusable objects where action and windows are starting
#define MENU_ACTION_OFF 8

//offset containing position and x size and +1
#define MENU_LISTPOS_OFF 5

//offset to the index of the checkbox and radio button
#define MENU_CKRAD_OFF 11

//offset for all options (2x screen XY cords)
#define MENU_CORDSKIP 6

#else

#define MENU_ACTION_OFF 6
#define MENU_LISTPOS_OFF 4
#define MENU_CKRAD_OFF 9
#define MENU_CORDSKIP 4

#endif



/*special window switch code for subwindow return
Even if this is a allowed address, a new window would never
start at the last Byte of the bytecode, so no collision is
possible.
*/
#ifdef USE16BITADDR

#define MENU_SUBWINDOW_RET 0xFFFF

#else

#define MENU_SUBWINDOW_RET 0xFFFFFF

#endif

void menu_redraw(void);
void menu_keypress(unsigned char key);

#ifdef MENU_MOUSE_SUPPORT
void menu_mouse(SCREENPOS x, SCREENPOS y, unsigned char key);
#endif


#endif
