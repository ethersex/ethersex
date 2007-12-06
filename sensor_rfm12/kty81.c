/* Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */


#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "kty81.h"

#define TEMPWERTSTART 452
#define TEMPWERTINTERVAL 4
int16_t PROGMEM temperaturwert[133] = { -304  ,
  -292  ,
  -280  ,
  -268  ,
  -256  ,
  -244  ,
  -232  ,
  -220  ,
  -208  ,
  -196  ,
  -184  ,
  -172  ,
  -160  ,
  -148  ,
  -136  ,
  -124  ,
  -112  ,
  -100  ,
  -88 ,
  -76 ,
  -64 ,
  -52 ,
  -40 ,
  -28 ,
  -17 ,
  -5  ,
  7 ,
  19  ,
  31  ,
  43  ,
  55  ,
  66  ,
  78  ,
  90  ,
  102 ,
  114 ,
  125 ,
  137 ,
  149 ,
  161 ,
  172 ,
  184 ,
  196 ,
  208 ,
  219 ,
  231 ,
  243 ,
  254 ,
  266 ,
  278 ,
  290 ,
  301 ,
  313 ,
  324 ,
  336 ,
  348 ,
  359 ,
  371 ,
  383 ,
  394 ,
  406 ,
  417 ,
  429 ,
  441 ,
  452 ,
  464 ,
  476 ,
  487 ,
  499 ,
  510 ,
  522 ,
  534 ,
  545 ,
  557 ,
  568 ,
  580 ,
  592 ,
  603 ,
  615 ,
  626 ,
  638 ,
  650 ,
  661 ,
  673 ,
  685 ,
  697 ,
  708 ,
  720 ,
  732 ,
  744 ,
  755 ,
  767 ,
  779 ,
  791 ,
  803 ,
  815 ,
  827 ,
  839 ,
  851 ,
  863 ,
  875 ,
  887 ,
  900 ,
  912 ,
  924 ,
  936 ,
  949 ,
  961 ,
  974 ,
  987 ,
  999 ,
  1012  ,
  1025  ,
  1038  ,
  1051  ,
  1064  ,
  1077  ,
  1090  ,
  1104  ,
  1117  ,
  1131  ,
  1144  ,
  1158  ,
  1172  ,
  1186  ,
  1200  ,
  1215  ,
  1229  ,
  1244  ,
  1259  ,
  1274  ,
  1289  ,
  1304
};


int16_t
temperatur(uint16_t sensorwert){
  
  uint16_t pos1 = (sensorwert - TEMPWERTSTART)/TEMPWERTINTERVAL;
  uint16_t pos2 = pos1 + 1;
  if(pos1 > 0 && pos2 < sizeof(temperaturwert)/2){
#ifdef PROGMEM
    int16_t tpos1 = pgm_read_word(temperaturwert+pos1);
    int16_t tpos2 = pgm_read_word(temperaturwert+pos2);
#else
    int16_t tpos1 = temperaturwert[pos1];
    int16_t tpos2 = temperaturwert[pos2];
#endif
    return (tpos1+((tpos2 - tpos1)/TEMPWERTINTERVAL)*(sensorwert - ((pos1*TEMPWERTINTERVAL) + TEMPWERTSTART)));
  }
  else{
    return(-1000);
  }
}

void 
temp2text(char *textbuf, int16_t temperatur){
  if (temperatur > -1000 && temperatur < 2000){
    snprintf(textbuf, 5, "%4i", temperatur);
    textbuf[4] = textbuf[3];
    textbuf[3] = '.';
  }
  else{
    sprintf(textbuf, "Out!!");
  }
}
