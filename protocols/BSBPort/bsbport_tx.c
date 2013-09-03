/*
 *
 * Copyright (c) 2008 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"
#include "bsbport.h"
#include "bsbport_helper.h"
#include "bsbport_tx.h"

uint8_t step=0;

// Low-Level sending of message to bus
uint8_t bsbport_send(uint8_t* msg) {

  uint8_t len = msg[3];
  msg[SOT] = 0xDC;
  msg[SRC] = 0x80 | BSBPORT_OWNADDRESS;

  {
    uint16_t crc = bsbport_crc(msg, len -2);
    msg[len -2] = (crc >> 8);
    msg[len -1] = (crc & 0xFF);
  }
  return bsbport_txstart(msg,msg[LEN]);
}


// High-level sending to bus
uint8_t bsbport_query(uint8_t A1, uint8_t A2, uint8_t A3, uint8_t A4, uint8_t dest) {
  uint8_t msg[11];
  msg[LEN] = 11;
  msg[TYPE] = QUERY;
  msg[DEST] = dest;
  // Adress switch A1,A2 for a query
  msg[P2] = A1;
  msg[P1] = A2;
  msg[P3] = A3;
  msg[P4] = A4;
  return bsbport_send(msg);
}

// High-level sending to bus
uint8_t bsbport_set(uint8_t A1, uint8_t A2, uint8_t A3, uint8_t A4, uint8_t dest, uint8_t* data, uint8_t datalen) {
  uint8_t msg[datalen+11];
  msg[LEN] = datalen+11;
  msg[TYPE] = SET;
  msg[DEST] = 0x80 & dest;
  // Adress 
  msg[P1] = A1;
  msg[P2] = A2;
  msg[P3] = A3;
  msg[P4] = A4;
  
  for (uint8_t i=DATA;i<msg[LEN]-2;i++)
  {
	msg[i] = data[i-DATA];
  }
  
  return bsbport_send(msg);
}

#ifdef BSBPORT_POLLING
// Query for values at periodic intervalls
void
bsbport_tx_periodic(void)
{
	switch( step )
    {
		// Temperaturen
        case 0 : 	//	Kollektortemperatur
			bsbport_query(0x49,0x3d,0x05,0x2a,0x00);
			step++;
            break;

        case 1 : 	//	SolarVorlauftemperatur
			bsbport_query(0x49,0x3d,0x05,0x0f,0x00);
			step++;
            break;

		case 2 : 	//	SolarRücklauftemperatur
			bsbport_query(0x49,0x3d,0x05,0x0e,0x00);
			step++;
            break;

		// Ertrag
        case 3 : 	//	Tagesertrag Solarenergie 
			bsbport_query(0x49,0x3d,0x05,0x99,0x00);
			step++;
	        break;

		// Sollwerte
		case 4 : 	//	Kompfortsollwert
			bsbport_query(0x2d,0x3d,0x05,0x8e,0x00);
			step++;
            break;

        case 5 : 	//	Reduziertsollwert 
			bsbport_query(0x2d,0x3d,0x05,0x90,0x00);
			step++;
	        break;

		case 6 : 	//	Trinkwassersollwert 
			bsbport_query(0x31,0x3d,0x06,0xb9,0x00);
			step++;
	        break;

		case 7 : 	//	Trinkwasserreduziertsollwert 
			bsbport_query(0x31,0x3d,0x06,0xba,0x00);
			step++;
	        break;
			
		default  : step = 0;
    }
}

/*
  -- Ethersex META --
  header(protocols/BSBPort/bsbport_tx.h)
  timer(500, bsbport_tx_periodic())
*/
#endif
thersex META --
  header(protocols/BSBPort/bsbport_tx.h)
  timer(500, bsbport_tx_periodic())
*/
#endif
