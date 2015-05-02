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


#include "menu-interpreter.h"
#include "menu-text.h"
#include "core/bit-macros.h"

#include "stdlib.h"

#ifdef DEBUG
#include "stdio.h"
#endif

//variables for dynamic data
unsigned char * menu_strings[MENU_TEXT_MAX];
unsigned char menu_checkboxstate[MENU_CHECKBOX_MAX];
unsigned char menu_radiobuttonstate[MENU_RADIOBUTTON_MAX];
unsigned short menu_listindexstate[MENU_LIST_MAX];
unsigned char * menu_gfxdata[MENU_GFX_MAX];

//the state of the menu
//the adress of the current window
MENUADDR menu_window_init = 0; //the drawn window
MENUADDR menu_subwindow_start = 0; //subwindow start adress, or 0 if no subwindow is shown
MENUADDR menu_window_start = 1; //the window which should be drawn
//the number of objects on the current window
unsigned char menu_objects;

unsigned char menu_focus_key_next, menu_focus_key_prev, menu_key_enter;
//the nuber of the object with the focus
unsigned char menu_focus;
unsigned char menu_focus_prime; //saves the focus of a window if the subwindo is active
unsigned char menu_focus_restore; //set to 1 if focus should be resored
//the index of all objects in the list, 0 for objects which disallow a focus
MENUADDR menu_focus_objects[MENU_OBJECTS_MAX];

MENUADDR menu_pc; //shows on the byte to read next
void menu_pc_set(MENUADDR addr) {
	menu_pc = addr;
}
unsigned char menu_byte_get_next(void) {
	return menu_byte_get(menu_pc++);
}
void menu_pc_skip(MENUADDR add) {
	menu_pc += add;
}

MENUADDR menu_assemble_addr(void) {
#ifdef USE16BITADDR
	return menu_byte_get_next() + (menu_byte_get_next() <<8);
#else
	return menu_byte_get_next() + (menu_byte_get_next() <<8) + (menu_byte_get_next() << 16);
#endif
}

MENUADDR menu_object_datasize(unsigned char object_id) {
	switch(object_id) {
		case MENU_BOX: return MENU_BOX_DATA;
		case MENU_LABEL: return MENU_LABEL_DATA;
		case MENU_BUTTON: return MENU_BUTTON_DATA;
		case MENU_GFX: return MENU_GFX_DATA;
		case MENU_LIST: return MENU_LIST_DATA;
		case MENU_CHECKBOX: return MENU_CHECKBOX_DATA;
		case MENU_RADIOBUTTON: return MENU_RADIOBUTTON_DATA;
		case MENU_SUBWINDOW: return MENU_SUBWINDOW_DATA;
		case MENU_WINDOW: return MENU_WINDOW_DATA;
		case MENU_SHORTCUT: return MENU_SHORTCUT_DATA;
		default: return 0;
	}
}

unsigned char menu_draw_Xline(SCREENPOS px, SCREENPOS py,
                              SCREENPOS length, unsigned char color) {
	SCREENPOS x;
	for (x = px; x < px+length; x++) {
		menu_screen_set(x, py, color & 1);
		if (color & 2) //invert lsb if 2. bit is set.
			color = ~color | 0x02;
	}
	return color;
}

unsigned char menu_draw_Yline(SCREENPOS px, SCREENPOS py,
                              SCREENPOS length, unsigned char color) {
	SCREENPOS y;
	for (y = py; y < py+length; y++) {
		menu_screen_set(px, y, color & 1);
		if (color & 2) //invert lsb if 2. bit is set.
			color = ~color | 0x02;
	}
	return color;
}

void menu_draw_border(SCREENPOS px, SCREENPOS py, SCREENPOS sx,
                      SCREENPOS sy, unsigned char color, unsigned char hasfocus) {
	if (hasfocus)
		color |= 0x02;
	unsigned char col2 = menu_draw_Xline(px, py, sx, color);
	if (col2 & 2) //invert lsb if 2. bit is set.
		col2 = ~col2 | 0x02;
	menu_draw_Yline(px+sx-1, py, sy, col2);
	color = menu_draw_Yline(px, py, sy, color);
	if (color & 2) //invert lsb if 2. bit is set.
		color = ~color | 0x02;
	menu_draw_Xline(px, py+sy-1, sx, color);
}

void menu_draw_box(SCREENPOS px, SCREENPOS py, SCREENPOS sx,
                   SCREENPOS sy, unsigned char color) {
	SCREENPOS x, y;
	SCREENPOS ex = px+sx;
	SCREENPOS ey = py+sy;
	for (y = py; y < ey; y++) {
		for (x = px; x < ex; x++) {
			menu_screen_set(x, y, color);
		}
	}
}

unsigned char menu_text_byte_get(MENUADDR baseaddr, unsigned short index,
                                 unsigned char storage) {
unsigned char c;
	if (storage == 0) {
		c = menu_byte_get(baseaddr+index);
	} else {
		if (baseaddr < MENU_TEXT_MAX) {
			if (menu_strings[baseaddr] != NULL) {
				c = menu_strings[baseaddr][index];
			} else
				c = '\0';
		} else
			c = '\0';
	}
	return c;
}

void menu_text_draw_base(SCREENPOS x, SCREENPOS y, unsigned char font,
                         unsigned char storage, MENUADDR baseaddr,
                         unsigned short offset, SCREENPOS maxpix) {
	unsigned char cdraw;
	unsigned short index = offset;
	maxpix += x;
	while (index < 10000) {
		cdraw = menu_text_byte_get(baseaddr, index, storage);
		index++;
		if ((cdraw == '\0') || (cdraw == '\n') || (x >= maxpix)) {
			break;
		}
		SCREENPOS ox = x;
		x += menu_char_draw(x, y, font, cdraw)+1;
		if (ox > x) {
			break; //prevent overflow on the right side of the screen back to the left
		}
	}
}

void menu_text_draw(SCREENPOS x, SCREENPOS y, unsigned char font,
 unsigned char storage, MENUADDR baseaddr) {
	menu_text_draw_base(x, y, font, storage, baseaddr, 0, MENU_SCREEN_X - x);
}

void menu_box(unsigned char hasfocus) {
#ifdef MENU_USE_BOX
	SCREENPOS px = menu_byte_get_next();
	SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get_next();
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	SCREENPOS sx = menu_byte_get_next();
	SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lsxy = menu_byte_get_next();
	sx += (SCREENPOS)(lsxy & 0xF0) << 4;
	sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
	menu_pc_skip(3+MENU_ADDR_BYTES); //options + action + windowaddr
	unsigned char color = menu_byte_get_next();
#ifdef DEBUG
	printf("Drawing box %i;%i with size %i;%i, color %i, focus %i\n", px, py, sx, sy, color, hasfocus);
#endif
	menu_draw_box(px, py, sx, sy, color);
	if (hasfocus) //append dotted border
		menu_draw_border(px, py, sx, sy, color, hasfocus);
#else
#ifdef DEBUG
	printf("Error: Box used, but not compiled in\n");
#endif
#endif
}

void menu_label(void) {
#ifdef MENU_USE_LABEL
	SCREENPOS px = menu_byte_get_next();
	SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get_next();
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	unsigned char storage = (menu_byte_get_next()>>MENU_OPTIONS_STORAGE) & 1;
	MENUADDR textaddr  = menu_assemble_addr();
	unsigned char fonts = menu_byte_get_next();
#ifdef DEBUG
	printf("Drawing label %i, %i with font %i, storage %i, addr %i\n", px, py, fonts, storage, textaddr);
#endif
	menu_text_draw(px, py, fonts, storage, textaddr);
#else
#ifdef DEBUG
	printf("Error: Label used, but not compiled in\n");
#endif
#endif
}

static void menu_basicbutton(unsigned char hasfocus, unsigned char offx, unsigned char offy) {
	SCREENPOS px = menu_byte_get_next();
	SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get_next();
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	SCREENPOS sx = menu_byte_get_next();
	SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lsxy = menu_byte_get_next();
	sx += (SCREENPOS)(lsxy & 0xF0) << 4;
	sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
	unsigned char options = menu_byte_get_next();
	menu_pc_skip(2+MENU_ADDR_BYTES); //action + windowaddr
	unsigned char storage = (options>>MENU_OPTIONS_STORAGE) & 1;
	MENUADDR textaddr  = menu_assemble_addr();
	unsigned char fonts = menu_byte_get_next();
#ifdef DEBUG
	printf("Drawing button %i, %i with font %i, storage %i, addr %i\n", px, py, fonts, storage, textaddr);
#endif
	if (hasfocus) {
		fonts = HI4(fonts);
	} else
		fonts = LO4(fonts);
	menu_text_draw(px+offx, py+offy, fonts, storage, textaddr);
	if (options & (1 << MENU_OPTIONS_RECTANGLE))
		menu_draw_border(px, py, sx, sy, 1, hasfocus);
}

void menu_button(unsigned char hasfocus) {
#ifdef MENU_USE_BUTTON
	//possible improvement: calculate second two parameters so that the text is ćentered
	menu_basicbutton(hasfocus, 2, 2);
#else
#ifdef DEBUG
	printf("Error: Button used, but not compiled in\n");
#endif
#endif
}

void menu_checkbox(unsigned char hasfocus) {
#ifdef MENU_USE_CHECKBOX
	SCREENPOS px = menu_byte_get(menu_pc);
	SCREENPOS py = menu_byte_get(menu_pc+1);
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get(menu_pc+2);
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	menu_basicbutton(hasfocus, 10, 0);
	unsigned char ckbnumber = menu_byte_get_next();
	menu_draw_border(px, py, 8, 7, 1, 0);
	unsigned char color = 0;
	if (hasfocus)
		color = 1;
	menu_draw_Xline(px, py+7, 8, color);
	color = 0;
	if ((ckbnumber < MENU_CHECKBOX_MAX) && (menu_checkboxstate[ckbnumber]))
		color = 1;
#ifdef DEBUG
	printf("Drawing checkbox %i with state %i\n", ckbnumber, color);
#endif
	//TODO: make the pattern available as gfx instead of hard coded values
	menu_screen_set(px+1, py+4, color);
	menu_screen_set(px+2, py+5, color);
	menu_screen_set(px+3, py+4, color);
	menu_screen_set(px+4, py+3, color);
	menu_screen_set(px+5, py+2, color);
	menu_screen_set(px+6, py+1, color);
#else
#ifdef DEBUG
	printf("Error: Checkbox used, but not compiled in\n");
#endif
#endif
}

void menu_radiobutton(unsigned char hasfocus) {
#ifdef MENU_USE_RADIOBUTTON
	SCREENPOS px = menu_byte_get(menu_pc);
	SCREENPOS py = menu_byte_get(menu_pc+1);
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get(menu_pc+2);
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	menu_basicbutton(hasfocus, 10, 0);
	unsigned char radionumber = menu_byte_get_next();
	unsigned char radioselect = HI4(radionumber);
	menu_draw_border(px, py, 8, 7, 1, 0);
	unsigned char color = 0;
	if (hasfocus)
		color = 1;
	menu_draw_Xline(px, py+7, 8, color);
	color = 0;
	if ((radionumber < MENU_RADIOBUTTON_MAX) && (menu_radiobuttonstate[radionumber] == radioselect))
		color = 1;
#ifdef DEBUG
	printf("Drawing radiobutton of group %i, checked on %i. Tableentry: %i\n", radionumber, radioselect, menu_checkboxstate[radionumber]);
#endif
	//TODO: make the pattern available as gfx instead of hard coded values
	menu_screen_set(px+3, py+2, color);
	menu_screen_set(px+4, py+2, color);
	menu_screen_set(px+2, py+3, color);
	menu_screen_set(px+3, py+3, color);
	menu_screen_set(px+4, py+3, color);
	menu_screen_set(px+5, py+3, color);
	menu_screen_set(px+3, py+5, color);
	menu_screen_set(px+4, py+5, color);
	menu_screen_set(px+2, py+4, color);
	menu_screen_set(px+3, py+4, color);
	menu_screen_set(px+4, py+4, color);
	menu_screen_set(px+5, py+4, color);
#else
#ifdef DEBUG
	printf("Error: Radiobutton used, but not compiled in\n");
#endif
#endif
}

void menu_gfx(unsigned char hasfocus) {
#ifdef MENU_USE_GFX
	SCREENPOS px = menu_byte_get_next();
	SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get_next();
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	SCREENPOS sx = menu_byte_get_next();
	SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lsxy = menu_byte_get_next();
	sx += (SCREENPOS)(lsxy & 0xF0) << 4;
	sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
	unsigned char options = menu_byte_get_next();
	menu_pc_skip(2+MENU_ADDR_BYTES); //action + windowaddr
	MENUADDR gfxaddr  = menu_assemble_addr();
	MENUADDR gfxinc = 0;
	unsigned char color = 0;
	unsigned char compressed = (options >> MENU_OPTIONS_COMPRESSED) & 1;
	unsigned char storage = (options >> MENU_OPTIONS_STORAGE) & 1;
	unsigned char havedata = 0;
	unsigned char data = 0;
	SCREENPOS x, y;
#ifdef DEBUG
	printf("Drawing gfx at %i, %i, compressed: %i, storage: %i, address: %i\n", px, py, compressed, storage, gfxaddr);
#endif
	for (y = py; y < py+sy; y++) {
		for (x = px; x < px+sx; x++) {
			//check if we have source data
			if (havedata == 0) { //fetch a next byte
				if (storage == 0) { //internal one
					data = menu_byte_get(gfxaddr+gfxinc);
				} else //if within the RAM
					data = menu_gfxdata[gfxaddr][gfxinc];
				//printf("%i\n", data);
				gfxinc++;
				if (compressed == 1) {
					havedata = data & 0x7F;
					color = data >> 7;
				} else
					havedata = 8;
			}
			if (compressed == 0) {
				if (data & 0x80) {
					color = 1;
				} else
					color = 0;
				data = data << 1;
			}
			havedata--;
			//printf("%i, %i: %i from %i\n", x, y, color, data);
			menu_screen_set(x, y, color);
		}
	}
	if ((hasfocus) || (options & (1<<MENU_OPTIONS_RECTANGLE))) //append dotted border
		menu_draw_border(px, py, sx, sy, 1, hasfocus);
#else
#ifdef DEBUG
	printf("Error: Gfx used, but not compiled in\n");
#endif
#endif
}

static unsigned short menu_list_lines(MENUADDR addr, unsigned char storage) {
	unsigned short lines = 0;
	unsigned short i = 0;
	unsigned char c;
	while ((c = menu_text_byte_get(addr, i++, storage)) != '\0') {
		if (c == '\n')
			lines++;
	}
	if (i) //count single string as one line too
		lines++;
	return lines;
}

static unsigned short menu_list_line_seek(MENUADDR addr, unsigned char storage, unsigned short line) {
	unsigned short i = 0;
	unsigned char c;
	while (line > 0) {
		c = menu_text_byte_get(addr, i, storage);
		if (c == '\n')
			line--;
		if (c == '\0')
			break;
		i++;
	};
	return i;
}

static unsigned short menu_list_line_next(MENUADDR baseaddr, unsigned short textpos, unsigned char storage) {
	unsigned char c;
	while(1) {
		c = menu_text_byte_get(baseaddr, textpos, storage);
		if (c == '\n') {
			textpos++;
			break;
		}
		if (c == '\0')
			break;
		textpos++;
	};
	return textpos;
}

void menu_list(unsigned char hasfocus) {
#ifdef MENU_USE_LIST
	//this is a very complex object
	SCREENPOS px = menu_byte_get_next();
	SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lpxy = menu_byte_get_next();
	px += (SCREENPOS)(lpxy & 0xF0) << 4;
	py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
	SCREENPOS sx = menu_byte_get_next();
	SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
	unsigned char lsxy = menu_byte_get_next();
	sx += (SCREENPOS)(lsxy & 0xF0) << 4;
	sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
	unsigned char options = menu_byte_get_next();
	menu_pc_skip(2+MENU_ADDR_BYTES); //action + windowaddr
	MENUADDR baseaddr  = menu_assemble_addr();
	unsigned char fonts = menu_byte_get_next();
	unsigned char listnumber = menu_byte_get_next();
	menu_pc_skip(4); //the four keys
	unsigned char storage = (options>>MENU_OPTIONS_STORAGE) & 1;
	SCREENPOS x, y;
#ifdef DEBUG
	printf("Drawing list %i;%i with size %i;%i\n", px, py, sx, sy);
#endif
	//draw the text
	unsigned short textlines = menu_list_lines(baseaddr, storage); //lines of the list to display
	unsigned short selectedline = menu_listindexstate[listnumber];
	SCREENPOS fontheight = menu_font_heigth(LO4(fonts))+1;
	SCREENPOS linesonscreen = (sy - 3)/ (fontheight); //maximum lines which could be displayed
	/*remove the -1 if on lists which can display even number of lines, should put 3 unselected lines
	 *before* the selected one, not after
	*/
	unsigned short beginningline = selectedline - ((linesonscreen-1) / 2);
	if ((selectedline >= textlines) && (textlines)) {
			/* if textlines is zero, the selected line is not important anyway */
		 menu_listindexstate[listnumber] = selectedline = textlines;
	}
	if ((beginningline+linesonscreen) > textlines) { //crop lower end
		if (textlines > linesonscreen) { //only crop, if more lines than there is to be drawn
			beginningline = textlines-linesonscreen;
		} else //draw from the beginning, if more screen lines than lines to draw
			beginningline = 0;
	}
	if (selectedline < ((unsigned short)(linesonscreen-1) / 2)) { //for the -1 see text above
		beginningline = 0;
	}
#ifdef DEBUG
	printf("textlines %i, selectedline %i, fontheigth %i, on screen %i, beginningline %i\n",
	  textlines, selectedline, fontheight, linesonscreen, beginningline);
#endif
	unsigned short textpos = menu_list_line_seek(baseaddr, storage, beginningline);
	y = py+2;
	unsigned short t;
	for (t = beginningline; t < (beginningline+linesonscreen); t++) { //now draw each line
		x = px +2;
		unsigned char font = LO4(fonts);
		if (t == selectedline)
			font = HI4(fonts);
		if (menu_text_byte_get(baseaddr, textpos, storage) == '\0')
			break;
#ifdef DEBUG
		printf("textpos %i\n", textpos);
#endif
		menu_text_draw_base(x, y, font, storage, baseaddr, textpos, sx-9);
		textpos = menu_list_line_next(baseaddr, textpos, storage);
		y += menu_font_heigth(font)+1;
	}
	//draw the surrounding
	menu_draw_border(px, py, sx, sy, 1, hasfocus);
	menu_draw_Yline(px+sx-6, py+1, sy-2, 1);
	menu_draw_box(px+sx-5, py+1, 4, sy-2, 0); //clear overlapped text
	//calculate right side scroller size and position
	SCREENPOS relpos, height;
	if (textlines > 0) {
		//wont overflow, because textlines > beginningline
		//+textlines/2 just helps to reduce rounding errors
		relpos = (((unsigned short)(sy-2))*beginningline+textlines/2)/textlines;
		unsigned short bheight = (((unsigned short)(sy-2))*linesonscreen+textlines/2)/textlines;
		if (bheight > (unsigned short)sy-2) {//no lager bars, than list height
			height = sy-2;
		} else
			height = bheight; //height may be only 8 bit width
	} else {
		relpos = 0;
		height = sy-2;
	}
	if (height == 0) //make at least a scrollbar with one pixel size
		height = 1;
#ifdef DEBUG
	printf("relpos %i, height %i\n", relpos, height);
#endif
	SCREENPOS abspos = py+1+relpos;
	if ((abspos+height) > (py+sy-1)) //limit lowest scrollbar position
		abspos = py+sy-1-height;
	menu_draw_box(px+sx-5, abspos, 4, height, 1);
#else
#ifdef DEBUG
	printf("Error: List used, but not compiled in\n");
#endif
#endif
}

void menu_shortcut(void) {
	//non visible object
	menu_pc_skip(MENU_SHORTCUT_DATA);
}

void menu_new_window(MENUADDR addr) {
	menu_pc_set(addr);
	unsigned char token = menu_byte_get_next();
	if (token == MENU_SUBWINDOW) {
		menu_pc_skip(MENU_CORDSKIP); //the coordinates
		//save old focus, if last window was a normal one
		if (menu_byte_get(menu_window_init) == MENU_WINDOW)
			menu_focus_prime = menu_focus;
	}
	if ((token == MENU_WINDOW) || (token == MENU_SUBWINDOW)) {
		menu_focus_key_prev = menu_byte_get_next();
		menu_focus_key_next = menu_byte_get_next();
		menu_key_enter = menu_byte_get_next();
		unsigned char index = 0;
		unsigned char options;
		while (index < MENU_OBJECTS_MAX) {
			token = menu_byte_get_next();
			MENUADDR t = menu_pc + menu_object_datasize(token);
			menu_focus_objects[index] = 0; //for LABEL and shortcut
			switch(token) {
				case MENU_BOX:
				case MENU_BUTTON:
				case MENU_GFX:
				case MENU_LIST:
				case MENU_CHECKBOX:
				case MENU_RADIOBUTTON:
						options = menu_byte_get(menu_pc+MENU_CORDSKIP); //offset for the options in all objects
						if (options & (1<<MENU_OPTIONS_FOCUSABLE)) { //if focusable
							menu_focus_objects[index] = menu_pc-1; //start addr of object
						} else
							menu_focus_objects[index] = 0;
 						break;
				case MENU_WINDOW:
				case MENU_SUBWINDOW:
				case MENU_INVALID: goto windowend; //sorry, no double break possible
				default: ; //do nothing for the label
			}
			menu_pc_set(t);
			index++; //count amounts of objects
			//define SCHEDULE if you use a multi tasking OS and the drawing take too long
			MENU_SCHEDULE
		}
		windowend:
		menu_objects = index;
#ifdef DEBUG
		printf("Objects in Window: %i\n", menu_objects);
#endif
		//check if old focus should be used
		if ((menu_focus_restore) && (menu_focus_objects[menu_focus_prime])) {
			menu_focus = menu_focus_prime;
			menu_focus_restore = 0;
		} else {
			//set up focus on first focusabel object
			menu_focus = 0;
			for (index = 0; index < menu_objects; index++) {
				if (menu_focus_objects[index] != 0) {
					menu_focus = index;
					break;
				}
			}
		}
#ifdef DEBUG
	} else {
		printf("Error: token %i is invalid for a new window\n", token);
#endif
	}
#ifdef DEBUG
	printf("new_window called: First focus: %i\n", menu_focus);
#endif
	menu_window_init = addr;
}

//draws two times in the case of a window with a sub window
void menu_draw_windowcontent(unsigned char focusallow) {
	unsigned char index = 0;
	while (index <= MENU_OBJECTS_MAX) {
		unsigned char token = menu_byte_get_next();
		unsigned char hasfocus = 0;
		if ((index == menu_focus) && (menu_focus_objects[index]) && (focusallow)) {
			hasfocus = 1;
		}
		switch(token) {
			case MENU_BOX: menu_box(hasfocus); break;
			case MENU_LABEL: menu_label(); break;
			case MENU_BUTTON: menu_button(hasfocus); break;
			case MENU_GFX: menu_gfx(hasfocus); break;
			case MENU_LIST: menu_list(hasfocus); break;
			case MENU_CHECKBOX: menu_checkbox(hasfocus); break;
			case MENU_RADIOBUTTON: menu_radiobutton(hasfocus); break;
			case MENU_SHORTCUT: menu_shortcut(); break;
			default: return;
		}
		index++; //count amounts of objects
	}
}

void menu_redraw(void) {
	//skip over possible global shortcuts if menu_window_init = 0
	if (menu_window_init == 0) { //first start
		unsigned char token = menu_byte_get(menu_window_start);
		while (token == MENU_SHORTCUT) {
			//should only happen at the beginning for global shortcuts
			menu_window_start += menu_object_datasize(MENU_SHORTCUT)+1;
			token = menu_byte_get(menu_window_start);
		}
	}
#ifdef DEBUG
	printf("window init: %i, subwindow start: %i, window start: %i\n", menu_window_init, menu_subwindow_start, menu_window_start);
#endif
	menu_screen_clear();
	if (menu_subwindow_start) {
		if (menu_window_init != menu_subwindow_start) {
			menu_new_window(menu_subwindow_start);
		}
	} else if (menu_window_init != menu_window_start) {
		menu_new_window(menu_window_start);
	}
	//draw window
	menu_pc_set(menu_window_start);
	unsigned char token = menu_byte_get_next();
	if (token == MENU_WINDOW)	{
		unsigned char allowfocus = 1;
		if (menu_subwindow_start)
			allowfocus = 0;
		menu_pc_skip(MENU_WINDOW_DATA);
		menu_draw_windowcontent(allowfocus);
#ifdef DEBUG
	} else {
		printf("Error: Not a window\n");
#endif
	}
	//draw subwindow, if enabled
	if (menu_subwindow_start) {
		menu_pc_set(menu_subwindow_start);
		token = menu_byte_get_next();
		if (token == MENU_SUBWINDOW) { //checking could be optimized out
			SCREENPOS px = menu_byte_get_next();
			SCREENPOS py = menu_byte_get_next();
#ifdef LARGESCREEN
			unsigned char lpxy = menu_byte_get_next();
			px += (SCREENPOS)(lpxy & 0xF0) << 4;
			py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
			SCREENPOS sx = menu_byte_get_next();
			SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
			unsigned char lsxy = menu_byte_get_next();
			sx += (SCREENPOS)(lsxy & 0xF0) << 4;
			sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
			menu_draw_box(px, py, sx, sy, 0);
			menu_draw_border(px, py, sx, sy, 1, 0);
			menu_pc_skip(3); //three byte for the keys
			menu_draw_windowcontent(1);
#ifdef DEBUG
		} else {
			printf("Error: Not a subwindow\n");
#endif
		}
	}
	menu_screen_flush();
}

void menu_run_action(MENUADDR addr) {
	menu_pc_set(addr);
#ifdef DEBUG
	printf("Reading action from adress %i\n", addr);
#endif
	unsigned short action = menu_byte_get_next() + (menu_byte_get_next() << 8);
	MENUADDR news = menu_assemble_addr();
	unsigned char wishredraw = menu_action(action);
	if (news != 0) { //if new window switch
		if (news == MENU_SUBWINDOW_RET) { //special value used for subwindow close
			menu_subwindow_start = 0;
			menu_focus_restore = 1;
		} else {
			unsigned char wtype = menu_byte_get(news);
			if (wtype == MENU_SUBWINDOW) { //look if new window is a subwindow or normal one
				menu_subwindow_start = news;
			} else {
				menu_window_start = news;
				menu_subwindow_start = 0;
			}
		}
	}
	if (news || wishredraw)
		menu_redraw();
}

#ifdef MENU_USE_CHECKBOX

static void menu_handle_checkbox(MENUADDR addr) {
	unsigned char index = menu_byte_get(addr+MENU_CKRAD_OFF+2*MENU_ADDR_BYTES);
	if (index < MENU_CHECKBOX_MAX)
		menu_checkboxstate[index] = 1-menu_checkboxstate[index];
	menu_redraw(); //Improvement: May be faster if only the checkbox gets redrawn
}

#endif

#ifdef MENU_USE_RADIOBUTTON

static void menu_handle_radiobutton(MENUADDR addr) {
	unsigned char groupindex = menu_byte_get(addr+MENU_CKRAD_OFF+2*MENU_ADDR_BYTES);
	unsigned char value = HI4(groupindex);
	if (groupindex < MENU_RADIOBUTTON_MAX)
		menu_radiobuttonstate[groupindex] = value;
	menu_redraw(); //Improvement: May be faster if only the radiobutton gets redrawn
}

#endif

static void menu_handle_listbox(MENUADDR addr, unsigned char key) {
	menu_pc_set(addr+MENU_LISTPOS_OFF);
	SCREENPOS sy = menu_byte_get_next();
#ifdef LARGESCREEN
			unsigned char lsxy = menu_byte_get_next();
			sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
	unsigned char options = menu_byte_get_next();
	unsigned char storage = (options>>MENU_OPTIONS_STORAGE) & 1;
	menu_pc_skip(2+MENU_ADDR_BYTES); //action + windowaddr
	MENUADDR baseaddr  = menu_assemble_addr();
	unsigned char fonts = menu_byte_get_next();
	unsigned short listindex = menu_byte_get_next();
	SCREENPOS fontheight = menu_font_heigth(LO4(fonts))+1;
	SCREENPOS linesonscreen = (sy - 3)/ (fontheight);
	if (listindex < MENU_LIST_MAX) { //if index is valid
		unsigned short nvalue = menu_listindexstate[listindex];
		unsigned short ovalue = nvalue;
		if (menu_byte_get_next() == key) {
			nvalue++;
		}
		if (menu_byte_get_next() == key) {
			nvalue--;
		}
		if (menu_byte_get_next() == key) {
			nvalue += linesonscreen;
		}
		if (menu_byte_get_next() == key) {
			nvalue -= linesonscreen;
		}
		if (ovalue != nvalue) { //a quick pre-checking
			unsigned short textlines = menu_list_lines(baseaddr, storage);
			if (nvalue > 32000) //indicates an underflow
				nvalue = 0;
			if (nvalue >= textlines) //indicaes an overflow
				nvalue = textlines-1;
			if (nvalue != menu_listindexstate[listindex]) {
				menu_listindexstate[listindex] = nvalue;
				menu_action(65535-1-listindex);
				menu_redraw();
			}
		}
	}
}

void menu_keypress(unsigned char key) {
	if (key == 0)
		return;
	//look if key is the enter key
	MENUADDR focusaddr = menu_focus_objects[menu_focus];
	if (focusaddr) { //if there is at least one object with a focus
		unsigned char obj = menu_byte_get(focusaddr);
		//look if this is a listbox
#ifdef MENU_USE_LIST
		if (obj == MENU_LIST) {
			menu_handle_listbox(focusaddr, key);
		}
#endif
		if (key == menu_key_enter) {
#ifdef MENU_USE_CHECKBOX
			if (obj == MENU_CHECKBOX)
				menu_handle_checkbox(focusaddr);
#endif
#ifdef MENU_USE_RADIOBUTTON
			if (obj == MENU_RADIOBUTTON)
				menu_handle_radiobutton(focusaddr);
#endif
			menu_run_action(focusaddr+MENU_ACTION_OFF); //offset for all focusable objects where action and windows are starting
			return;
		}
	}
	//look if key swiches the focus
	if (key == menu_focus_key_prev) {
		unsigned char i;
		for (i = 0; i < menu_objects; i++) { //seek next object which is focusable
			menu_focus--;
			if (menu_focus >= menu_objects)
				menu_focus = menu_objects-1;
			if (menu_focus_objects[menu_focus] != 0) //found one
				break;
		}
#ifdef DEBUG
		printf("New focus: %i\n", menu_focus);
#endif
		menu_redraw();
		return;
	}
	if (key == menu_focus_key_next) {
		unsigned char i;
		for (i = 0; i < menu_objects; i++) { //seek next object which is focusable
			menu_focus++;
			if (menu_focus >= menu_objects)
				menu_focus = 0;
			if (menu_focus_objects[menu_focus] != 0) //found one
				break;
		}
#ifdef DEBUG
		printf("New focus: %i\n", menu_focus);
#endif
		menu_redraw();
		return;
	}
	//look if key is part of a shortcut
	unsigned char windowtoken = menu_byte_get(menu_window_init);
	MENUADDR p = menu_window_init+menu_object_datasize(windowtoken)+1;
	unsigned char token;
	while (1) {
		token = menu_byte_get(p);
		if (token == MENU_SHORTCUT) {
			if (key == menu_byte_get(p+1)) {
#ifdef DEBUG
				printf("Found proper shortcut at %i, key: %i\n", p, key);
#endif
				menu_run_action(p+2);
				return;
			} //else
				//printf("Found other shortcut at %i with key %i\n", p, menu_byte_get(p+1));
		} else if ((token == MENU_WINDOW) || (token == MENU_SUBWINDOW) || (token == MENU_INVALID)) {
			break;
		}
		p += menu_object_datasize(token)+1;
	}
  //look if key is part of a global shortcut
	p = 1;
	while (1) {
		token = menu_byte_get(p);
		if (token == MENU_SHORTCUT) {
			if (key == menu_byte_get(p+1)) {
#ifdef DEBUG
				printf("Found proper global shortcut at %i, key: %i\n", p, key);
#endif
				menu_run_action(p+2);
				return;
			} //else
				//printf("Found other shortcut at %i with key %i\n", p, menu_byte_get(p+1));
		} else {
			break;
		}
		p += menu_object_datasize(token)+1;
	}
}

#ifdef MENU_MOUSE_SUPPORT

void menu_mouse(SCREENPOS x, SCREENPOS y, unsigned char key) {
	MENUADDR elemmatch = 0;
	MENUADDR seekpos = 0;
	//check if sub window active or not
	if (menu_subwindow_start) {
		seekpos = menu_subwindow_start;
	} else
		seekpos = menu_window_start;
	//jump over first window definitions
	seekpos += menu_object_datasize(menu_byte_get(seekpos))+1;
	//check objects, remember last found object
	while(1) {
		unsigned char obj = menu_byte_get(seekpos);
		if ((obj == MENU_BOX) || (obj == MENU_BUTTON) || (obj == MENU_GFX) ||
		    (obj == MENU_LIST) || (obj == MENU_CHECKBOX) || (obj == MENU_RADIOBUTTON)) {
			MENUADDR spos2 = seekpos+1;
			SCREENPOS px = menu_byte_get(spos2++);
			SCREENPOS py = menu_byte_get(spos2++);
#ifdef LARGESCREEN
			unsigned char lpxy = menu_byte_get(spos2++);
			px += (SCREENPOS)(lpxy & 0xF0) << 4;
			py += (SCREENPOS)(lpxy & 0x0F) << 8;
#endif
			SCREENPOS sx = menu_byte_get(spos2++);
			SCREENPOS sy = menu_byte_get(spos2++);
#ifdef LARGESCREEN
			unsigned char lsxy = menu_byte_get(spos2++);
			sx += (SCREENPOS)(lsxy & 0xF0) << 4;
			sy += (SCREENPOS)(lsxy & 0x0F) << 8;
#endif
			unsigned char focusable = menu_byte_get(spos2) & 1;
			if (focusable) {
				//compare if position fits
				if ((x >= px) && (x < (px+sx)) && (y >= py) && (y < (py+sy))) {
					elemmatch = seekpos;
				}
			}
		} else if ((obj != MENU_LABEL) && (obj != MENU_SHORTCUT)) {
				break;
		}
		seekpos += menu_object_datasize(obj)+1;
	}
	//run action
	if (elemmatch) {
		unsigned char obj = menu_byte_get(elemmatch);
#ifdef MENU_USE_LIST
		if (obj == MENU_LIST) {
			menu_handle_listbox(elemmatch, key);
		}
#endif
		if (key == menu_key_enter) {
#ifdef MENU_USE_CHECKBOX
			if (obj == MENU_CHECKBOX)
				menu_handle_checkbox(elemmatch);
#endif
#ifdef MENU_USE_RADIOBUTTON
			if (obj == MENU_RADIOBUTTON)
				menu_handle_radiobutton(elemmatch);
#endif
			menu_run_action(elemmatch+MENU_ACTION_OFF); //offset for all focusable objects where action and windows are starting
			return;
		}
	}
}

#endif


