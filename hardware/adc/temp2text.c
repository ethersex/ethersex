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

#include <stdlib.h>
#include <avr/pgmspace.h>
#include "hardware/adc/temp2text.h"

/* gibt die Temperatur (in Zehntelgrad) formatiert als Klartext
 * im Textbuffer zurueck.
 * Mindestlaenge des buf ist 6 byte
 * Aequivalent zu:
 *   sprintf(textbuf, "% 3i.%1i", temperatur/10, abs(temperatur%10));
 */
void
temp2text(char *textbuf, int16_t temperatur)
{
  if (temperatur > -300 && temperatur < 1500){
    char *ptr = textbuf;

    /* fülle mit Padding-bytes auf */
    if (temperatur > -100 && temperatur < 1000)
      *ptr++ = ' ';
    if (temperatur > -10 && temperatur < 100)
      *ptr++ = ' ';
    if (temperatur >= 0 && temperatur < 10)
      *ptr++ = '0';

    itoa (temperatur, ptr, 10);

    /* konvertiere Zehntelgrad nach Grad: baue Dezimalstelle */
    textbuf[4] = textbuf[3];
    textbuf[3] = ',';

    textbuf[5] = '\0';
  }
  else{
    /* "Out of range" Indikator */
    strcpy_P(textbuf, PSTR("!NaN!"));
  }
  // return 5; <-- maybe better make it explicit
}
