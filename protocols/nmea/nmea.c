/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "nmea.h"

struct nmea_t nmea_data;

#include "config.h"
#define USE_USART NMEA_USE_USART
#define BAUD 4800
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

void
nmea_init(void)
{
  /* Initialize the usart module */
  usart_init();
}


ISR(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE))))
    {
      uint8_t v = usart(UDR);
      (void) v;
      return;
    }

  uint8_t data = usart(UDR);
  if (data == '$')
    {
      nmea_data.locked = 1;
      nmea_data.ptr = 0;
    }
  else if (!nmea_data.locked)
    return;
  else
    nmea_data.ptr ++;

  if ((nmea_data.ptr == 3 || nmea_data.ptr == 4)
      && data != 'G')
  streamerror: {
      nmea_data.locked = 0;
      return;
    }

  if (nmea_data.ptr == 5 && data != 'A')
    goto streamerror;

  if (nmea_data.ptr >= 18 && nmea_data.ptr <= 26)
    nmea_data.latitude[nmea_data.ptr - 18] = data;

  else if (nmea_data.ptr == 28)
    nmea_data.latitude_dir = data;

  else if (nmea_data.ptr >= 30 && nmea_data.ptr <= 39)
    nmea_data.longitude[nmea_data.ptr - 30] = data;

  else if (nmea_data.ptr == 41)
    nmea_data.longitude_dir = data;

  else if (nmea_data.ptr == 45)
    nmea_data.satellites = (data - '0') * 10;

  else if (nmea_data.ptr == 46)
    {
      nmea_data.satellites += (data - '0');
      nmea_data.valid = 1;
    }
}


/*
  -- Ethersex META --
  header(protocols/nmea/nmea.h)
  init(nmea_init)
*/
