/*
Copyright (C) 2009 Stefan Riepenhausen <rhn@gmx.net> (changes for ethersex)
Copyright (C) 2009 Sonja Waringer <4711ontour@gmail.com>
Copyright (C) pre2009 http://www.das-labor.org 
Original codebase SVN: 
svn co https://roulette.das-labor.org/svn/microcontroller/src-atmel/borg/borg-16
This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "mcuf/mcuf_modul.h"
#ifdef MCUF_MODUL_BORG16_XONI_STUDY_SUPPORT


void draw_queue(uint8_t x, uint8_t y, uint8_t on);

void xoni_study1(void) {
	uint8_t i, x,y,z=1;
	for(i=0;i<6;i++) {
		for(y=0; y<16; y++) {
			for(x=0; x<16; x++){
				draw_queue((z?15-x:x),y, 1);
				WAIT(42);
				draw_queue((z?15-x:x),y, 0);
			}
			z ^= 1;
		}
		for(y=15; y; y--) {
			setpixel((pixel){y,y},3);
			WAIT(42);
			setpixel((pixel){y-1,y-1},3);
			WAIT(42);
			setpixel((pixel){y,y},0);			
		}
	}
}

void draw_queue(uint8_t x, uint8_t y, uint8_t on) {
	setpixel((pixel){x,y},on?3:0);
	
	if (y % 2) {
		// wir laufen nach links
		if (x>1) {
			//easy
			setpixel((pixel){x-1,y},on?2:0);
			setpixel((pixel){x-2,y},on?1:0);
		} else if (x == 1) {
			//Rand!!
			setpixel((pixel){0,y},on?2:0);
			setpixel((pixel){0,y-1},on?1:0);			
		} else /*if (x == 0)*/ {
			setpixel((pixel){0,y-1},on?2:0);
			setpixel((pixel){1,y-1},on?1:0);
		}
	} else {
		// wir laufen nach rechts
		if (x<14) {
			//easy
			setpixel((pixel){x+1,y},on?2:0);
			setpixel((pixel){x+2,y},on?1:0);
		} else if (x == 14) {
			//Rand!!
			setpixel((pixel){15,y},on?2:0);
			setpixel((pixel){15,y-1},on?1:0);			
		} else {
			setpixel((pixel){15,y-1},on?2:0);
			setpixel((pixel){14,y-1},on?1:0);
		}
	}
}
#endif  //MCUF_MODUL_BORG16_XONI_STUDY_SUPPORT
