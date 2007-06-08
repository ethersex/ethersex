/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
#include "ecmd/ecmd.h"

#define noinline __attribute__((noinline))

/* prototypes */
void uip_log(char *message);
int debug_uart_put(char d, FILE *stream);

#ifdef DEBUG

void DEBUG_INIT_UART()
/* {{{ */ {

    /* set baud rate */
    _UBRRH_UART0 = HI8(DEBUG_UART_UBRR);
    _UBRRL_UART0 = LO8(DEBUG_UART_UBRR);

    /* set mode */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0);

    /* open stdout/stderr */
    fdevopen(debug_uart_put, NULL);

} /* }}} */

int noinline debug_uart_put(char d, FILE *stream)
/* {{{ */ {

    if (d == '\n')
        debug_uart_put('\r', stream);

    while (!(_UCSRA_UART0 & _BV(_UDRE_UART0)));
    _UDR_UART0 = d;

    return 0;

} /* }}} */

void DEBUG_PROCESS_UART(void)
/* {{{ */ {
#define LEN 60
#define OUTPUTLEN 40

    static char buf[LEN+1];
    static char *ptr = buf;

    if (_UCSRA_UART0 & _BV(_RXC_UART0)) {
        char data = _UDR_UART0;

        if (data == '\n' || data == '\r') {
            char *output = malloc(OUTPUTLEN);

            if (output == NULL)
                debug_printf("malloc() failed!\n");

            *ptr = '\0';
            printf_P(PSTR("\n"));

            debug_printf("parsing command '%s'\n", buf);

            int l = ecmd_parse_command(buf, output, LEN);
            if (l > 0)
                printf_P(PSTR("%s\n"), output);
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

} /* }}} */

void uip_log(char *message)
/* {{{ */ {

    debug_printf("uip: %s\n", message);

} /* }}} */

#endif
