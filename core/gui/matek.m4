#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "gui.h"
#include "font.h"

#include "config.h"

static uint8_t color;

void (*matek_selected_scene)(struct gui_block *) = NULL;

void
matek_draw(struct gui_block *dest) {
    if (matek_selected_scene) 
        matek_selected_scene(dest);
}

divert(-1)
ifelse(ARCH_AVR, y, define(`_pgm', 1))
define(`global_divert', 1)
define(`graphical_divert', 2)
define(`text_divert', 3)
define(`screne_end_divert', 4)
define(`PUSHDIVERT', `pushdef(`old_divert', divnum)divert($1)') 
define(`POPDIVERT', `divert(old_divert)popdef(`old_divert')')

define(`ROW2BLOCK', `(($1) / 2)')
define(`COL2BLOCK', `((($1) * GUI_FONT_WIDTH) / GUI_BLOCK_WIDTH)')

dnl BUFFER(name, len)
define(`BUFFER', `PUSHDIVERT(global_divert)char $1[$2];POPDIVERT()')

dnl PUTSTRING(data, x, y, w, h)
dnl x,w   columns not blocks
dnl y,h   rows not blocks
define(`PUTSTRING', `ifelse(substr(`$1', 0, 1), `"', `define(`_pgm', `1')', `define(`_pgm', 0)')
  if (dest->y >= ROW2BLOCK($3) && dest->y <= ROW2BLOCK($3 + $5) 
      && dest->x >= COL2BLOCK($2)  && dest->x <= COL2BLOCK($2 + $4)) {
	    ifelse(_pgm, `1', `char *data = PSTR(`$1');')
		uint8_t x, y;
		for (y = 0; y < $5; y++) 
		    for (x = 0; x < $4; x++)
			    if ((y * $4 + x) < strlen(`$1'))  
			        gui_putchar(dest, ifelse(_pgm, `1', `pgm_read_byte(&data[y * $4 + x])', `((char*)$1)[y * $4 + x]'), color, $3 + y, $2 + x); 
  }')

define(`SCENE', `divert(graphical_divert)

void
matek_scene_$1(struct gui_block *dest) 
{')
define(`SCENE_END', `}divert(0)undivert')

