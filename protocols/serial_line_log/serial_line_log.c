/*
 * Copyright (c) 2009, 2010 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"

#ifdef DEBUG_SERIAL_LINE_LOG
# include "core/debug.h"
# define SLL_DEBUG(a...)  debug_printf("sll: " a)
#else
# define SLL_DEBUG(a...)
#endif

#include "pinning.c"
#include "serial_line_log.h"

#define USE_USART SERIAL_LINE_LOG_USE_USART
#define BAUD SERIAL_LINE_LOG_BAUDRATE
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init ()
struct serial_line_log_data sll_data_new;
struct serial_line_log_data sll_data;

void serial_line_log_init (void)
{
  /* Initialize the usart module */
  usart_init ();
}

void
serial_line_log_periodic (void)
{
  if (sll_data.timeout)
    sll_data.timeout--;

}

ISR (usart (USART, _RX_vect))
{
  /* Ignore errors */
  if ((usart (UCSR, A) & _BV (usart (DOR)))
      || (usart (UCSR, A) & _BV (usart (FE))))
    {
      uint8_t v = usart (UDR);
      (void) v;
      return;
    }

  uint8_t data = usart (UDR);

#ifdef SERIAL_LINE_LOG_SPACE_COMPRESSION
  static uint8_t last_was_space;
  if (data == ' ' || data == '\t')
    {
      if (last_was_space)
	return;
      last_was_space = 1;
      data = ' ';
    }
  else
    last_was_space = 0;
#endif /* SPACE Compression */

  if (data == ((uint8_t *) SERIAL_LINE_LOG_EOL)[0])
    {
      /* Yeah we have reached end of line so commit it to the real
         buffer */
      if (sll_data_new.len > 1)
	{
	  memcpy (&sll_data, &sll_data_new, sizeof (sll_data_new));

	  /* Set the timeout for the new data correct */
	  sll_data.timeout = SERIAL_LINE_LOG_TIMEOUT;

	  /* Termiate the Buffer correct */
	  sll_data.data[sll_data.len++] = 0;
	}

      sll_data_new.len = 0;

      SLL_DEBUG ("got new line, line was stored\n");
      return;
    }
  else if (sll_data_new.len >= SERIAL_LINE_LOG_COUNT)
    {
      /* Our Buffer is more than full, with correct configuration
         should this never happen */
      sll_data.len = sprintf_P ((char *) sll_data.data, PSTR ("overrun"));
      SLL_DEBUG ("buffer overrun\n");
      sll_data_new.len = 0;
    }
  sll_data_new.data[sll_data_new.len++] = data;
}

/*
  -- Ethersex META --
  header(protocols/serial_line_log/serial_line_log.h)
  init(serial_line_log_init)
  timer(50, serial_line_log_periodic())
*/
