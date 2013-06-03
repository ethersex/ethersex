/*
 * Copyright (c) 2021 by Frank Sautter <ethersix@sautter.com>
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

#include <avr/io.h>
#include <stdio.h>

#include "config.h"

#ifdef ELTAKOMS_SUPPORT

#include "eltakoms.h"
#include "services/clock/clock.h"

#define USE_USART ELTAKOMS_USE_USART
#define BAUD 19200
#include "core/usart.h"

struct eltakoms_t eltakoms_data;


/* We generate our own usart init module, for our usart port */
generate_usart_init()

void eltakoms_init(void)
{
#if !RS485_HAVE_TE
  #warning no RS485 transmit enable pin for ELTAKOMS defined
#endif

  RS485_TE_SETUP;               // configure RS485 transmit enable as output
  RS485_DISABLE_TX;             // disable RS485 transmitter
  usart_init();

  ELTAKOMS_DEBUG("init\n");
}

ISR(usart(USART, _RX_vect))
{
  /* ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE))))
  {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }

  uint8_t data = usart(UDR);

  if (data == 'W' ||          // start of frame
      eltakoms_data.ptr >= sizeof(eltakoms_data.buffer))
    eltakoms_data.ptr = 0;

  if (data != 0x03)           // end of text
  {
    eltakoms_data.buffer[eltakoms_data.ptr++] = data;
    return;
  }

  eltakoms_data.buffer[eltakoms_data.ptr] = '\0';
  ELTAKOMS_DEBUG("%s\n", eltakoms_data.buffer);

  int16_t sum = 0;
  uint8_t i;

  for ( i = 0; i < 39; i++)
  {
    data = eltakoms_data.buffer[i];

    if (i < 35)
      sum += data;

    switch (i)
    {
    case 0:
      if (data != 'W')
        goto streamerror;
      break;
    case 1:
      if (data != '+' && data != '-' )
        goto streamerror;
      break;
    case 4:
    case 18:
      if (data != '.')
        goto streamerror;
      break;
    case 21:
      if (data != '?' && (data <  '1' || data >  '7'))
        goto streamerror;
      break;
    case 34:
      if (data != '?' && (data != 'J' && data != 'N' ))
        goto streamerror;
      break;
    case 12:
    case 20:
      if (data != 'J' && data != 'N' )
        goto streamerror;
      break;
    default:
      if (data <  '0' || data >  '9')
        goto streamerror;
      break;
    }
  }

  if (atoi(&eltakoms_data.buffer[ELTAKOMS_CHECKSUM]) != sum)
  streamerror: {
    ELTAKOMS_DEBUG("%d - 0x%02x - %c\n", i, data, data);
    eltakoms_data.ptr = 0;
    return;
  }

  eltakoms_data.rain = (eltakoms_data.buffer[ELTAKOMS_RAIN] == 'J');

  eltakoms_data.buffer[ELTAKOMS_WIND + 2] =
    eltakoms_data.buffer[ELTAKOMS_WIND + 3];
  eltakoms_data.buffer[ELTAKOMS_WIND + 3] = '\0';
  eltakoms_data.wind = atoi(&eltakoms_data.buffer[ELTAKOMS_WIND]);

  eltakoms_data.buffer[ELTAKOMS_WIND] = '\0';
  eltakoms_data.dawn = atoi(&eltakoms_data.buffer[ELTAKOMS_DAWN]);

  eltakoms_data.obscure = (eltakoms_data.buffer[ELTAKOMS_OBSCURITY] == 'J');

  eltakoms_data.buffer[ELTAKOMS_OBSCURITY] = '\0';
  eltakoms_data.sune = atoi(&eltakoms_data.buffer[ELTAKOMS_SUNEAST]);

  eltakoms_data.buffer[ELTAKOMS_SUNEAST] = '\0';
  eltakoms_data.sunw = atoi(&eltakoms_data.buffer[ELTAKOMS_SUNWEST]);

  eltakoms_data.buffer[ELTAKOMS_SUNWEST] = '\0';
  eltakoms_data.suns = atoi(&eltakoms_data.buffer[ELTAKOMS_SUNSOUTH]);

  eltakoms_data.buffer[ELTAKOMS_TEMP + 3] =
    eltakoms_data.buffer[ELTAKOMS_TEMP + 4];
  eltakoms_data.buffer[ELTAKOMS_TEMP + 4] = '\0';
  eltakoms_data.temperature = atoi(&eltakoms_data.buffer[ELTAKOMS_TEMP]);

  eltakoms_data.ts = clock_get_time();
  eltakoms_data.valid = 1;

  ELTAKOMS_DEBUG("t%+3.3d s%2.2d w%2.2d e%2.2d %c d%3.3d v%3.3d %c\n",
    eltakoms_data.temperature, eltakoms_data.suns, eltakoms_data.sunw,
    eltakoms_data.sune, (eltakoms_data.obscure ? 'O' : 'o'),
    eltakoms_data.dawn, eltakoms_data.wind, (eltakoms_data.rain ? 'R' : 'r'));
}

void eltakoms_periodic(void)
{
  if (eltakoms_data.valid == 1 && (eltakoms_data.ts + 10 < clock_get_time()))
  {
    eltakoms_data.valid = 0;  /* invalid data after 10 seconds no reception */
    ELTAKOMS_DEBUG("data invalid\n");
  }
}
#endif /* ELTAKOMS_SUPPORT */

/*
  -- Ethersex META --
  header(protocols/eltakoms/eltakoms.h)
  init(eltakoms_init)
  timer(50, eltakoms_periodic())

*/
