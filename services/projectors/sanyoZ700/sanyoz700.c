/*
* Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#include "sanyoz700.h"

#include <stdint.h>
#include <string.h>
#include "core/debug.h"

#define USE_USART SANYO_Z700_USE_USART
#define USE_2X 0
#define BAUD 19200
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

/* Initialize stella */
void
sanyoZ700_init (void)
{
	usart_init();
	/* Disable the receiver */
    //usart(UCSR,B) &= ~_BV(usart(RXCIE));
}

void sanyoZ700cmd(char cmd[2])
{
	#ifdef DEBUG_SANYO_Z700
		debug_printf("SANYO Z700 cmd: C%c%c %i\n", cmd[0], cmd[1], BAUD);
	#endif
	/* sanyo commands are 4 bytes, beginning with "C" and two hex chars ("0"-"9" + "A"-"F") and \r
	   Commands are defined in http://www.sanyo.com.au/support/instruction-manuals/plv-z700-basic-serial-command-functional-specifications.
	   Basic commands are: Power on: C00, Power off: C01, Lamp mute on: C0D, Lamp mute off: C0E, Eco on: C75, Eco off: C74*/
	while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = 'C';
	while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = cmd[0];
	while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = cmd[1];
	while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = '\r';
}

ISR(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
	#ifdef DEBUG_SANYO_Z700
	uint8_t data = usart(UDR);
		debug_printf("SANYO Z700 got: %c\n", data);
	#endif
}

/*
  -- Ethersex META --
  header(services/projectors/sanyoZ700/sanyoz700.h)
  init(sanyoZ700_init)
*/
