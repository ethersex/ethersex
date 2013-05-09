/*
Copyright (C) 2009 Stefan Riepenhausen <rhn@gmx.net> (changes for ethersex)
Copyright (C) pre2009 http://www.das-labor.org 
Original codebase SVN: 
svn co https://roulette.das-labor.org/svn/microcontroller/src-atmel/borg/borg-16
Copyright (c) pre2009 by Guido Pannenbecker <info@sd-gp.de>

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "mcuf/mcuf_modul.h"
#include <stdlib.h>
#ifdef MCUF_MODUL_BORG16_SNAKE_SUPPORT

#include "snake.h"

void snake(){
	pixel pixels[64];
	pixels[0] = (pixel){NUM_COLS/2, NUM_ROWS/2};
	pixels[1] = (pixel){NUM_COLS/2, (NUM_ROWS/2)-1};
	
	pixel * head = &pixels[1];
	pixel * tail = &pixels[0];
	pixel old_head;

	pixel apples[10];
	unsigned char apple_num = 0;
	
	direction dir = up;

	clear_screen(0);

	unsigned char x=0, dead=0;
	while(1){
		x++;
		old_head = *head;
		if(++head == pixels + 64) head = pixels;
		
		unsigned char dead_cnt=0;
		
		unsigned char apple_found = 0, j;
		for(j=0;j<apple_num;j++){
			unsigned char i;
			for(i=0;i<4;i++){
				if ( (next_pixel(old_head, i).x == apples[j].x) && (next_pixel(old_head, i).y == apples[j].y) ){
					apple_found = 1;
					dir = i;
					for(;j<apple_num-1;j++){
						apples[j]=apples[j+1];
					}
					apple_num--;
					goto apple_se;
				}
			}
		}
		apple_se:

		if(apple_found){
		
		}else{
			while(get_next_pixel(old_head, dir)){
				if((dead_cnt++)==4){
					dead = 1;
					break;
				}
				dir = direction_r(dir);
			}
		}

		if(!dead){
			*head = next_pixel(old_head, dir);
			setpixel(*head, 3);
		
			if((rand()&0xff)<80){
				unsigned char j;
				unsigned char nextapple=0, distx, disty, shortdist=255, xy=0;
				if(!apple_num){
					dir = rand()%4;
				}else{
					for(j=0;j<apple_num;j++){
						if(head->x > apples[j].x){
							distx = head->x - apples[j].x;
						}else{
							distx = apples[j].x - head->x;
						}
						if(head->y > apples[j].y){
							disty = head->y - apples[j].y;
						}else{
							disty = apples[j].y - head->y;
						}
						if ((distx + disty) < shortdist){
							shortdist = distx + disty;
							nextapple = j;
							xy = (distx > disty)?1:0;
						}
					}
					if(xy){
						dir = (apples[nextapple].x > head->x)?left:right;
					}else{
						dir = (apples[nextapple].y > head->y)?down:up;
					}
				}
			}

			if( (apple_num<9) && ((rand()&0xff)<10) ){
				pixel new_apple = (pixel){rand()%NUM_COLS,rand()%NUM_ROWS};
				if(!get_pixel(new_apple)){
					apples[apple_num++]=new_apple;
				}
			}

			if(!apple_found){
				clearpixel(*tail);
				if(++tail == pixels + 64) tail = pixels;
			}
		}else{
			while(tail != head){
				clearpixel(*tail);
				if((++tail)>pixels+64) tail = pixels;
				wait (60);
			}
			break;
		}
		
		for(j=0;j<apple_num;j++){
			if(x%2){
				setpixel(apples[j], 3);
			}else{
				clearpixel(apples[j]);
			}
		}
		
		wait (SNAKE_DELAY);
	}
}
#endif // MCUF_MODUL_BORG16_SNAKE_SUPPORT
