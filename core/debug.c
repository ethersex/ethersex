/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include "core/debug.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "hardware/lcd/s1d15g10/s1d15g10.h"

#define noinline __attribute__((noinline))


char *
debug_binary(uint8_t v)
{
  static char binstr[9];
  uint8_t i;

  binstr[8] = '\0';
  for (i = 0; i < 8; i++)
  {
    binstr[7 - i] = v & 1 ? '1' : '0';
    v = v / 2;
  }
  return binstr;
}


static FILE debug_uart_stream = FDEV_SETUP_STREAM (debug_uart_put, NULL, _FDEV_SETUP_WRITE);


/* prototypes */
void soft_uart_putchar(uint8_t c);

#ifdef DEBUG_USE_USART
  #define USE_USART DEBUG_USE_USART
#else
  #define USE_USART 0
#endif

#define BAUD DEBUG_BAUDRATE
#include "core/usart.h"
#include "pinning.c"

/* We generate our own usart init module, for our usart port */
generate_usart_init()


void
debug_init_uart(void)
{
#ifndef SOFT_UART_SUPPORT
  RS485_TE_SETUP;
  RS485_DISABLE_TX;
  usart_init();

  /* disable the receiver we just enabled */
  usart(UCSR, B) &= ~(_BV(usart(RXCIE)) | _BV(usart(RXEN)));
#endif
  /* open stdout/stderr */
  stdout = &debug_uart_stream;
  stderr = &debug_uart_stream;
}


int noinline
debug_uart_put(char d, FILE * stream)
{
  if (d == '\n')
    debug_uart_put('\r', stream);

  /* replace escape sequences */
  if (d == 0x1b)
    d = '^';

  #ifdef S1D15G10_SUPPORT
    lcd_putch(d);
  #endif /* S1D15G10_SUPPORT */
  #ifdef SOFT_UART_SUPPORT
    soft_uart_putchar(d);
  #else /* SOFT_UART_SUPPORT */
    while (!(usart(UCSR, A) & _BV(usart(UDRE))));

    #if RS485_HAVE_TE
      /* enable interrupt usart transmit complete */
      usart(UCSR,B) |= _BV(usart(TXCIE));
      RS485_ENABLE_TX;
    #endif  /* RS485_HAVE_TE */

    usart(UDR) = d;
  #endif /* SOFT_UART_SUPPORT */
  return 0;
}

/* interrupt routine to disable the RS485 transmitter */
#if RS485_HAVE_TE
  ISR(usart(USART,_TX_vect))
  {
    RS485_DISABLE_TX;
    /* disable interrupt usart transmit complete */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
  }
#endif  /* RS485_HAVE_TE */


void noinline
debug_uart_putstr(const char *d)
{
  while (*d != 0)
  {
    debug_putchar(*d++);
  }
}


void
debug_process_uart(void)
{
#if defined(ECMD_PARSER_SUPPORT) && !defined(SOFT_UART_SUPPORT)
#define LEN 60
#define OUTPUTLEN 40

  static char buf[LEN + 1];
  static char *ptr = buf;

  if (usart(UCSR, A) & _BV(usart(RXC)))
  {
    char data = usart(UDR);

    if (data == '\n' || data == '\r')
    {
      char *output = malloc(OUTPUTLEN);

      if (output == NULL)
        debug_printf("malloc() failed!\n");

      *ptr = '\0';
      printf_P(PSTR("\n"));

#ifdef DEBUG_ECMD
      debug_printf("parsing command '%s'\n", buf);
#endif
      int l;

      do
      {
        l = ecmd_parse_command(buf, output, OUTPUTLEN);
        if (is_ECMD_FINAL(l) || is_ECMD_AGAIN(l))
        {
          output[is_ECMD_AGAIN(l) ? ECMD_AGAIN(l) : l] = 0;
          printf_P(PSTR("%s\n"), output);
        }
      }
      while (is_ECMD_AGAIN(l));
      free(output);
      ptr = buf;
    }
    else
    {
      debug_uart_put(data, stdout);
      if (data == '\b')
      {
        if (ptr > &buf[0])
          ptr--;
      }
      else
      {
        if (ptr < &buf[LEN - 1])
          *ptr++ = data;
        else
          debug_printf("not enough space for storing '%c'\n", data);
      }
    }
  }
#endif /* ECMD_PARSER_SUPPORT && !SOFT_UART_SUPPORT */
}


/*
  -- Ethersex META --
  header(core/debug.h)
  mainloop(debug_process_uart)
*/
