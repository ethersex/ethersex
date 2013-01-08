/*
 * Copyright (c) 2012 by Gerd v. Egidy <gerd@egidy.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void
usage(int exitval)
{
  fprintf(exitval ? stderr : stdout,
          "Usage: crc16-concat FILE\n"
          "Calculate a CRC16 of FILE and concatenate it to FILE.\n\n");
  exit(exitval);
}

// Polynomial: x^16 + x^15 + x^2 + 1 (0xa001)
// Initial value: 0xffff
// taken from the AVR libc documentation

uint16_t
crc16_update(uint16_t crc, uint8_t a)
{
  int i;

  crc ^= a;
  for (i = 0; i < 8; ++i)
  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }

  return crc;
}

int
main(int argc, char **argv)
{
  uint16_t crc = 0xffff;
  uint8_t b;
  FILE *f;

  if (argc == 2 && strcmp(argv[1], "--help") == 0)
    usage(0);
  if (argc != 2)
    usage(1);

  if ((f = fopen(argv[1], "r+b")) == NULL)
  {
    fprintf(stderr, "Error opening %s.\n", argv[1]);
    return 1;
  }

  // read file byte-by-byte, updating the crc

  while (fread(&b, 1, 1, f) == 1)
  {
    crc = crc16_update(crc, b);
  }
  if (ferror(f))
  {
    fprintf(stderr, "Error reading from %s.\n", argv[1]);
    fclose(f);
    return 1;
  }

  // concat the crc to the file
  fwrite(&crc, 2, 1, f);

  fclose(f);

  fprintf(stdout, "padding CRC16: %4X\n", crc);

  return 0;
}
