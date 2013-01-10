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
          "merges adc values got by 'adc get' and valculates the temperature from it (kty81-110)\n");
  fprintf(stderr, "./kty-merge [-c <kty-calibration-file>] [0-7]* [0-7]*\n");
}

int
get_calibration_value(int sensor_index, char *calibration_file)
{
  if (!calibration_file)
    return 2200;
  FILE *file = fopen(calibration_file, "r");
  if (!file)
  {
    fprintf(stderr, "Can't open calibration file\n");
    exit(EXIT_FAILURE);
  }
  char buf[4096];
  while (!feof(file))
  {
    int sensor, value;
    fgets(buf, 4095, file);
    if (sscanf(buf, "%d: %d", &sensor, &value) == 2)
    {
      if (sensor == sensor_index)
      {
        fclose(file);
        return value;
      }
    }
  }

  fclose(file);
  return 2200;
}


float
adc_to_temperature(int sensor, char *calibration_file, uint16_t adc)
{
  int R2K2 = get_calibration_value(sensor, calibration_file);
  float aT = -167.123;
  float bT = 0.275501;
  float cT = -0.000102316;
  float dT = 1.92025e-08;

  float volt = (2.5 / 1023) * adc;
  float R = R2K2 * adc / (2048.0 - adc);
  return aT + R * (bT + R * (cT + R * dT));

}

int
main(int argc, char *argv[])
{
  if (argc == 1)
  {
    usage();
    exit(EXIT_SUCCESS);
  }

  int i;
  int16_t adc[8];
  char *calibration_file = NULL;

  if (scanf("%x %x %x %x %x %x %x %x",
            &adc[0], &adc[1], &adc[2],
            &adc[3], &adc[4], &adc[5], &adc[6], &adc[7]) < 4)
  {
    fprintf(stderr, "The Input value might be corrupted");
    exit(EXIT_FAILURE);
  }

  /* Calibration file */
  i = 1;
  if (strcmp(argv[1], "-c") == 0)
  {
    calibration_file = argv[2];
    i += 2;
  }
  for (; i < argc; i++)
  {
    double average = 0;
    int values = 0;
    char *p = argv[i];
    while (*p)
      if (adc[*p - '0'] != 1023)
      {
        average +=
          adc_to_temperature(*p - '0', calibration_file, adc[*p - '0']);
        p++;
        values++;
      }
      else
        p++;
    if (average == 0)
    {
      fprintf(stderr, "No sensor values found\n");
      exit(EXIT_FAILURE);
    }

    average /= values;
    printf("%.2f\n", average);

  }




  /*  if (p[1] != 0xff && p[0] != 0x3f) {
   * printf("W: %d Sensor: %d C: %3.1f R: %.0f\n", adc, sensor, temperatur, R);
   * }
   * p += 3;
   * sensor++;
   * } */
  return 0;


}
