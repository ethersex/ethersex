/* vim:fdm=marker ts=4 ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include <stdio.h>
#include "config.h"
#include "debug.h"
#include "ecmd_parser/ecmd.h"
#include "onewire/onewire.h"

#define USE_USART 0
#define BAUD DEBUG_BAUDRATE
#include "usart.h"

#define noinline __attribute__((noinline))

/* prototypes */
void uip_log(char *message);
int debug_uart_put(char d, FILE *stream);

#ifdef DEBUG

#ifndef DEBUG_USE_SYSLOG

/* We generate our own usart init module, for our usart port */
generate_usart_init()

void
debug_init_uart (void)
{
    usart_init();

    /* Disable the receiver */
    usart(UCSR,B) &= ~_BV(usart(RXCIE));

    /* open stdout/stderr */
    fdevopen(debug_uart_put, NULL);
}


int noinline
debug_uart_put (char d, FILE *stream)
{
    if (d == '\n')
        debug_uart_put('\r', stream);

    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = d;

    return 0;
}

void
debug_process_uart (void)
{
#define LEN 60
#define OUTPUTLEN 40

    static char buf[LEN+1];
    static char *ptr = buf;

    if (usart(UCSR,A) & _BV(usart(RXC))) {
        char data = usart(UDR);

        if (data == '\n' || data == '\r') {
            char *output = malloc(OUTPUTLEN);

            if (output == NULL)
                debug_printf("malloc() failed!\n");

            *ptr = '\0';
            printf_P(PSTR("\n"));

#ifdef DEBUG_ECMD
            debug_printf("parsing command '%s'\n", buf);
#endif
            int l;

            do {
                l = ecmd_parse_command(buf, output, LEN);
                if (l > 0 || l < -10)
                    printf_P(PSTR("%s\n"), output);
            } while (l <= -10);
            free(output);
            ptr = buf;
        } else {
            debug_uart_put(data, stdout);

            if (ptr < &buf[LEN-1])
                *ptr++ = data;
            else
                debug_printf("not enough space for storing '%c'\n", data);
        }
    }
}

#endif	/* not DEBUG_USE_SYSLOG */


void uip_log(char *message)
{
    debug_printf("uip: %s\n", message);
}

#endif	/* DEBUG */
