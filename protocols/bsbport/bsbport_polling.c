/*
 *
 * Copyright (c) 2013-2014 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include <avr/io.h>
#include "config.h"
#include "bsbport_tx.h"
#include "bsbport_polling.h"

#ifdef BSBPORT_POLLING_SUPPORT
uint8_t step = 0;
// Query for values at periodic intervals
void
bsbport_polling_periodic(void)
{
  switch (step)
  {
      // Temperaturen
    case 0:                    //      Kollektortemperatur
      bsbport_query(0x49, 0x3d, 0x05, 0x2a, 0x00);
      step++;
      break;

    case 1:                    //      SolarRücklauftemperatur
      bsbport_query(0x49, 0x3d, 0x05, 0x0f, 0x00);
      step++;
      break;

    case 2:                    //      SolarVorlauftemperatur
      bsbport_query(0x49, 0x3d, 0x05, 0x0e, 0x00);
      step++;
      break;

    case 3:                    //      Trinkwassertemperatur 31        3d      05      2f
      bsbport_query(0x31, 0x3d, 0x05, 0x2f, 0x00);
      step++;
      break;

    case 4:                    //      Puffertemperatur 1 05   3d      05      34
      bsbport_query(0x05, 0x3d, 0x05, 0x34, 0x00);
      step++;
      break;

    case 5:                    //      Puffertemperatur 2 05   3d      05      35
      bsbport_query(0x05, 0x3d, 0x05, 0x35, 0x00);
      step++;
      break;

    case 6:                    //      HeizungsVorlauftemperatur
      bsbport_query(0x21, 0x3d, 0x05, 0x18, 0x00);
      step++;
      break;

    case 7:                    //      Puffertemperatur 3 05   3d      05      36
      bsbport_query(0x05, 0x3d, 0x05, 0x36, 0x00);
      step++;
      break;

      // Ertrag
    case 8:                    //      Tagesertrag Solarenergie 
      bsbport_query(0x49, 0x3d, 0x05, 0x99, 0x00);
      step++;
      break;

      // Sollwerte
    case 9:                    //      Kompfortsollwert 0x2d 0x3d 0x05 0x8e
      bsbport_query(0x2d, 0x3d, 0x05, 0x8e, 0x00);
      step++;
      break;

    case 10:                   //      Reduziertsollwert 
      bsbport_query(0x2d, 0x3d, 0x05, 0x90, 0x00);
      step++;
      break;

    case 11:                   //      Trinkwassersollwert 
      bsbport_query(0x31, 0x3d, 0x06, 0xb9, 0x00);
      step++;
      break;

    case 12:                   //      Trinkwasserreduziertsollwert 
      bsbport_query(0x31, 0x3d, 0x06, 0xba, 0x00);
      step++;
      break;

    case 13:                   //      Heizungsvorlaufsollwert 
      bsbport_query(0x21, 0x3d, 0x06, 0x67, 0x00);
      step++;
      break;

    case 14:                   //      Puffersollwert 
      bsbport_query(0x05, 0x3d, 0x08, 0x83, 0x00);
      step++;
      break;

    case 15:                   //      Kesselsollwert 
      bsbport_query(0x05, 0x3d, 0x07, 0x83, 0x00);
      step++;
      break;

      // Status
    case 16:                   //      Heizkreispumpe Q2  05   3d      09      A5
      bsbport_query(0x05, 0x3d, 0x09, 0xA5, 0x00);
      step++;
      break;

    case 17:                   //      Trinkwasserpumpe Q3  05 3d      09      A3
      bsbport_query(0x05, 0x3d, 0x09, 0xA3, 0x00);
      step++;
      break;

    case 18:                   //      Relaisausgang QX1  05   3d      09      BA -> Kollektorpumpe
      bsbport_query(0x05, 0x3d, 0x09, 0xBA, 0x00);
      step++;
      break;

    case 19:                   //      Relaisausgang QX2  05   3d      09      BB -> 
      bsbport_query(0x05, 0x3d, 0x09, 0xBB, 0x00);
      step++;
      break;

    case 20:                   //      Relaisausgang QX3  05   3d      09      BC -> 
      bsbport_query(0x05, 0x3d, 0x09, 0xBC, 0x00);
      step++;
      break;

    case 21:                   //      Relaisausgang QX4  05   3d      09      BD -> Umschaltung Solar Puffer
      bsbport_query(0x05, 0x3d, 0x09, 0xBD, 0x00);
      step++;
      break;

    case 22:                   //      Fehlerstatus            05      3d      00      9A 
      bsbport_query(0x05, 0x3d, 0x00, 0x9A, 0x00);
      step++;
      break;

    default:
      step = 0;
  }
}

/*
  -- Ethersex META --
  header(protocols/bsbport/bsbport_polling.h)
  ifdef(`conf_BSBPORT_POLLING',`timer(150, `bsbport_polling_periodic()')')
*/
#endif
