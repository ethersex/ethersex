 /*  Copyright(C) 2008 Christian Dietrich <stettberger@dokucode.de>
  * 
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


void
usage()
{
  fprintf(stderr,
          "calculates the 2K2 resistor, when a 1000R resistor is attached\n");
  fprintf(stderr, "./kty-calibrate [1000R Value] [0-7]*\n");
}

int
main(int argc, char *argv[])
{
  if (argc < 3)
  {
    usage();
    exit(EXIT_SUCCESS);
  }

  int i;
  int16_t adc[8];

  if (scanf("%x %x %x %x %x %x %x %x",
            &adc[0], &adc[1], &adc[2],
            &adc[3], &adc[4], &adc[5], &adc[6], &adc[7]) < 4)
  {
    fprintf(stderr, "The Input value might be corrupted");
    exit(EXIT_FAILURE);
  }
  int16_t R1000 = atoi(argv[1]);
  for (i = 2; i < argc; i++)
  {
    int16_t adc_counter = atoi(argv[i]);
    if (adc_counter > 7)
    {
      usage();
      exit(EXIT_FAILURE);
    }
    double R2K2 = R1000 * (2048.0 - adc[adc_counter]) / adc[adc_counter];
    printf("%d: %.0f\n", adc_counter, R2K2);
  }

  return 0;
}
