/* vim:fdm=marker ts=4 et ai
 * {{{
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
 }}} */

#ifndef GCC
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../config.h"
#include "../syslog/syslog.h"
#include "../clock/clock.h"
#include "mcuf.h"
#include "mcuf_net.h"
#include "mcuf_text.h"
#include "ledmatrixint.h"
#include "../uip/uip.h"
#else
#include "../contrib/mcuf/modsim.h"
#endif // GCC

#include "mcuf_modul.h"

#ifdef MCUF_MODUL_SUPPORT
#include "borg-16/xoni_study.h"
#include "borg-16/matrix.h"

#ifdef MCUF_MODUL_BORG16_SUPPORT

void setpixel(pixel p, uint8_t color){
  setPixel(p.x, p.y, color);
}
#endif //MCUF_MODUL_BORG16_SUPPORT

#ifdef MCUF_CHESS_SUPPORT
void clean(){
  mcuf_clean(0,1);
}
#endif
#ifdef MCUF_SPIRAL_SUPPORT
void spiral(){
  mcuf_spiral(5);
}
#endif

struct mcuf_modul_t mcuf_display_modules[] PROGMEM = 
{
#ifdef MCUF_CHESS_SUPPORT
{ mcuf_chess },
#endif
#ifdef MCUF_SPIRAL_SUPPORT
{ spiral },
#endif
#ifdef MCUF_CLEAN_SUPPORT
{ clean },
#endif
#ifdef MCUF_BORG16_MATIX_SUPPORT
{ matrix },
#endif
#ifdef MCUF_MODUL_BORG16_XONI_STUDY_SUPPORT
{ xoni_study1 },
#endif
{ NULL }
};

uint8_t mcuf_current_modul = 0;

uint8_t mcuf_play_modul(MCUF_PLAY_MODE play_mode, uint8_t modul)
{
#ifdef SYSLOG_SUPPORT
    syslog_send_P(PSTR("mcuf play modul"));
#endif


  switch (play_mode){
    case MCUF_MODUL_PLAY_MODE_MANUAL: 
      mcuf_current_modul = modul;
      break;
    case MCUF_MODUL_PLAY_MODE_SEQUENCE:
      mcuf_current_modul++;
      break;
    case MCUF_MODUL_PLAY_MODE_RANDOM: 
      mcuf_current_modul = rand() ;
      break;
  }
 
  struct mcuf_modul_t modulfunc;
  for (uint8_t i = 0; ; i++) {

    memcpy_P(&modulfunc, &mcuf_display_modules[i], sizeof(struct mcuf_modul_t));

    if (modulfunc.handler == NULL) break;

    if (mcuf_current_modul == i ) {
#ifdef SYSLOG_SUPPORT
    syslog_send_P(PSTR("mcuf play modul: %i"),i);
#endif
    	modulfunc.handler();
	break;
    }
  }
  return mcuf_current_modul;
}

#ifdef MCUF_CHESS_SUPPORT
void mcuf_chess()
{
  uint8_t y;
  uint8_t x;
  for (y=0; y < MCUF_MAX_SCREEN_WIDTH; y++)
    for (x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++)
       setPixel(x, y, x + y);
}
#endif //MCUF_CHESS_SUPPORT

#ifdef MCUF_CLEAN_SUPPORT
void mcuf_clean(uint8_t color, uint8_t delay)
{
  uint8_t y;
  uint8_t x;
  for (y=0; y < MCUF_MAX_SCREEN_WIDTH; y++){
    for (x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++) {
       setPixel(x, y, 0);
      _delay_ms(delay);
     }
  }
}
#endif //MCUF_CLEAN_SUPPORT

#ifdef MCUF_SPIRAL_SUPPORT
void mcuf_spiral(uint8_t delay)
{
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t x1 = 0;
  uint8_t y1 = 0;
  uint8_t x2 = MCUF_MAX_SCREEN_WIDTH-1;
  uint8_t y2 = MCUF_MAX_SCREEN_HEIGHT-1;

  while ( (x1 <= x2) && (y1 <= y2)){
    for (x=x1; x < x2; x++) {
      setPixel(x, y, 1);
      _delay_ms(delay);
    }
    x1++;
    for (y=y1; y < y2; y++){
      setPixel(x, y, 2);
      _delay_ms(delay);
    }
    y1++;
    for (x=x2; x > x1; x--) {
      setPixel(x, y, 3);
      _delay_ms(delay);
    }
    x2--;
    for (y=y2; y > y1; y--){
      setPixel(x, y, 1);
      _delay_ms(delay);
    }
    y2--;
  } 
}

#endif //MCUF_CLEAN_SUPPORT


#endif //MCUF_MODUL_SUPPORT

