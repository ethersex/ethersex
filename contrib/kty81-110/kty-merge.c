 /*  Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

  
void 
usage() 
{
  fprintf(stderr, "merges adc values got by 'adc get' and valculates the temperature from it (kty81-110)\n");
  fprintf(stderr, "./kty-merge [0-7]* [0-7]*\n");
}

float
adc_to_temperature(uint16_t adc) {
  float aT = -167.123;
  float bT = 0.275501;
  float cT = -0.000102316;
  float dT  = 1.92025e-08;

  float volt = (2.5/1023) * adc;
  float R = 2200/(5 - volt)*volt;
  R = 2180 * adc / (2048.0 - adc);
  return aT + R*( bT + R*( cT + R*dT));

}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    usage();
    exit(EXIT_SUCCESS);
  }

  int i;
  int16_t adc[8];

  if (scanf("%x %x %x %x %x %x %x", 
            &adc[0], &adc[1],&adc[2],
            &adc[3],&adc[4],&adc[5],
            &adc[6],&adc[7]) < 4) {
    fprintf(stderr, "The Input value might be corrupted");
    exit(EXIT_FAILURE);
  }
  for (i = 1; i < argc; i++) {
    long unsigned int average = 0;
    int values = 0;
    char *p = argv[i];
    while (*p) 
      if (adc[*p - '0'] != 1023) {
        average += adc[*p++ - '0'];
        values++;
      } else 
        p++;
    average /= values;
    printf("%.2f\n", adc_to_temperature(average));

  }




  /*  if (p[1] != 0xff && p[0] != 0x3f) {
      printf("W: %d Sensor: %d C: %3.1f R: %.0f\n", adc, sensor, temperatur, R);
    }
    p += 3;
    sensor++;
  } */
  return 0;


}
