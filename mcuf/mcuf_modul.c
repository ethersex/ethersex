/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Guido Pannenbecker <info@sd-gp.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
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

#ifndef GCC 
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "config.h"
#include "core/debug.h"
#include "services/clock/clock.h"
#include "mcuf.h"
#include "mcuf_net.h"
#include "mcuf_text.h"
#include "ledmatrixint.h"
#else
#include "contrib/mcuf/modsim.h"
#endif // GCC

#include "mcuf_modul.h"
#include "mcuf_modul_examples.h"
#include "image/image.h"

#ifdef MCUF_MODUL_SUPPORT

#ifdef MCUF_TEST_GAME_INPUT
#include "mcuf_modul_test_game_input.h"
#endif // MCUF_TEST_GAME_INPUT

// max number of available modules
#define MCUF_MAX_MODULES ((sizeof(mcuf_display_modules) / sizeof(struct mcuf_modul_t)) - 1)

#ifdef MCUF_MODUL_BORG16_SUPPORT
#include "mcuf/borg-16/xoni_study.h"
#include "mcuf/borg-16/snake.h"


void setpixel(pixel p, uint8_t color){
  setPixel(p.x, p.y, color);
}

void clear_screen(uint8_t color) {
  uint8_t y;
  uint8_t x;
  for (y=0; y < MCUF_MAX_SCREEN_WIDTH; y++)
    for (x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++)
       setPixel(x, y, color);
}

void set_cursor(cursor* cur, pixel p){
        cur->pos = p;
        switch (cur->mode){
                case clear:
                        clearpixel(p);
                        break;
                case set:
                        setpixel(p,3);
                        break;
        }
}

pixel next_pixel(pixel pix, direction dir){
        switch (dir){
                        case right:
                                return((pixel){pix.x-1, pix.y});
                                break;
                        case left:
                                return((pixel){pix.x+1, pix.y});
                                break;
                        case down:
                                return((pixel){pix.x, pix.y+1});
                                break;
                        case up:
                                return((pixel){pix.x, pix.y-1});
                                break;

                        }
        return (pixel){0,0};
}

void walk(cursor* cur, unsigned char steps, unsigned int delay){
        unsigned char x;
        for(x=0;x<steps;x++){
                set_cursor(cur, next_pixel(cur->pos, cur->dir));
                wait(delay);
        }
}

unsigned char get_pixel(pixel p) {
	return getPixel(p.x,p.y);
}
unsigned char get_next_pixel(pixel p, direction dir){
        pixel tmp;
        switch (dir){
                case right:
                        tmp = (pixel){p.x-1, p.y};
                        break;
                case left:
                        tmp = (pixel){p.x+1, p.y};
                        break;
                case down:
                        tmp = (pixel){p.x, p.y+1};
                        break;
                case up:
                        tmp = (pixel){p.x, p.y-1};
                        break;
                default:
                        tmp = p;
                        break;
        }
        return get_pixel(tmp);
}

direction direction_r(direction dir){
        switch (dir){
                        case right:
                                return(down);
                        case down:
                                return(left);
                        case left:
                                return(up);
                        case up:
                                return (right);
                }
        return(0);
}

#endif //MCUF_MODUL_BORG16_SUPPORT

#ifdef MCUF_CLEAN_SUPPORT
void clean(){
  mcuf_clean(0,1);
}
#endif
#ifdef MCUF_SPIRAL_SUPPORT
void spiral(){
  mcuf_spiral(5);
}
#endif

struct mcuf_modul_t {
 void (*handler)(void);
 PGM_P title;
};

#include "mcuf_modul_defs.c"

uint8_t mcuf_current_modul = 0;

uint8_t mcuf_list_modul(char *title, uint8_t modul){
  if ( modul >= MCUF_MAX_MODULES ) {
      return 0;
  } 
  struct mcuf_modul_t modulfunc;
  memcpy_P(&modulfunc, &mcuf_display_modules[modul], sizeof(struct mcuf_modul_t));
  memcpy_P(title,modulfunc.title, strlen_P(modulfunc.title) + 1);
#ifdef DEBUG_MCUF
//    debug_printf("mcuf modul: %i, %s\n",i, modulfunc.title);
#endif
  return 1;
}

uint8_t mcuf_play_modul(MCUF_PLAY_MODE play_mode, uint8_t modul)
{
  uint8_t i;
#ifdef DEBUG_MCUF
    debug_printf("mcuf play modul\n");
#endif


  switch (play_mode){
    case MCUF_MODUL_PLAY_MODE_MANUAL: 
      mcuf_current_modul = modul;
      break;
    case MCUF_MODUL_PLAY_MODE_SEQUENCE:
      mcuf_current_modul++;
      break;
    case MCUF_MODUL_PLAY_MODE_RANDOM: 
      mcuf_current_modul = (rand() & MCUF_MAX_MODULES);
      break;
  }
 
  struct mcuf_modul_t modulfunc;
  for (i = 0; ; i++) {

    memcpy_P(&modulfunc, &mcuf_display_modules[i], sizeof(struct mcuf_modul_t));

    if (modulfunc.handler ==  NULL) {
#ifdef DEBUG_MCUF
    debug_printf("mcuf play modul end\n");
#endif
       mcuf_current_modul = 0;
       break;
    }

    if (mcuf_current_modul == i ) {
#ifdef DEBUG_MCUF
    debug_printf("mcuf play modul: %i, %s\n",i, modulfunc.title);
#endif
    	modulfunc.handler();
	break;
    }
  }
  return mcuf_current_modul;
}

#endif //MCUF_MODUL_SUPPORT

