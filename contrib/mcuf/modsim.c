/*
 *
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

#ifndef GCC
  #error "no GCC compiler used"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "../../autoconf.h"
#include "modsim.h"
#include "../../mcuf/mcuf_modul.h"

char colors[4] = " .oX";

char mcuf_matrix[MCUF_MAX_SCREEN_WIDTH][MCUF_MAX_SCREEN_HEIGHT];

void setPixel(uint8_t x, uint8_t y, uint8_t color)
{

	mcuf_matrix[x][y]=color;

	system("clear");
	
	printf("\n");
	for (x=0;x<MCUF_MAX_SCREEN_HEIGHT;x++)
		printf("%2i",x);
	printf("\n");

	for (y=0;y<MCUF_MAX_SCREEN_WIDTH;y++){
		for (x=0;x<MCUF_MAX_SCREEN_HEIGHT;x++){
			putchar(colors[ mcuf_matrix[MCUF_MAX_SCREEN_WIDTH - x][y] % sizeof(colors) ]);
			putchar(' ');
		}
		printf("  %i\n",y);
	}
	printf("\n");

}

uint8_t getPixel(uint8_t x, uint8_t y){
	return mcuf_matrix[x][y];
}

void _delay_ms(uint8_t delay){
	usleep(delay*1000);
}

void initMatrix(void) {
	int x=0;
        int y=0;
	for (y=0;y<MCUF_MAX_SCREEN_WIDTH;y++){
		for (x=0;x<MCUF_MAX_SCREEN_HEIGHT;x++){
			mcuf_matrix[x][y]=0;
		}
	}
}

int
main(int argc, char *argv[])
{
  int i=0;
  char title[15];

  initMatrix();

  for (i = 0 ; i < 25; i++)
  {
	if (!mcuf_list_modul(title, i))
	  break;
  	printf("%i. mcuf_list_modul: %s\n", i, title);
  	mcuf_play_modul(MCUF_MODUL_PLAY_MODE_MANUAL,i);
  }

  printf("EOF\n");
  return 0;
}
