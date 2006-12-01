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

#include <avr/io.h>
#include "network.h"
#include "uart.h"

void uart_init(void)
/*{{{*/ {
    /* set baud rate */
    _UBRRH_UART0 = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    _UBRRL_UART0 = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    //_UCSRC_UART0 = UART_UCSRC;
    /* 9 bit framesize */
    _UCSRC_UART0 = _BV(UCSZ00) | _BV(UCSZ01);

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0) | _BV(UCSZ02);

} /* }}} */

void uart_putc(uint8_t data)
/* {{{ */ {

    while (!(_UCSRA_UART0 & _BV(_UDRE_UART0)));

    _UDR_UART0 = data;

} /* }}} */

void uart_putf(void *buffer)
/* {{{ */ {

    uint8_t c;
    uint8_t *b = (uint8_t *)buffer;

    while ((c = pgm_read_byte(b)) != '\0') {
        uart_putc(c);
        b++;
    }

} /* }}} */

void uart_puthexnibble(uint8_t data)
/* {{{ */ {

    if (data > 9) {
        data -= 10;
        uart_putc('A' + data);
    } else {
        uart_putc('0' + data);
    }

} /* }}} */

void uart_puthexbyte(uint8_t data)
/* {{{ */ {

    /* first high, then low nibble */
    uart_puthexnibble((data & 0xF0) >> 4);
    uart_puthexnibble(data & 0x0F);

} /* }}} */

void uart_putdecbyte(uint8_t data)
/* {{{ */ {

    uint8_t pot10[] = {100, 10};

    for (uint8_t i=0; i<2; i++) {

        uint16_t p = pot10[i];
        uint8_t digit = 48;

        while (data >= p) {
            digit++;
            data -= p;
        }

        uart_putc(digit);

    }

    uart_putc(data+48);

} /* }}} */

void uart_eol(void)
/* {{{ */ {
    uart_puts_P("\r\n");
} /* }}} */

void uart_puts_mac(struct uip_eth_addr *mac)
/* {{{ */ {

#ifdef DEBUG
    for (uint8_t i = 0; i < 6; i++)
        uart_puthexbyte(mac->addr[i]);
#endif

} /* }}} */

void uart_puts_ip(uip_ipaddr_t *ip)
/* {{{ */ {

    uint8_t *byte = (uint8_t *)ip;

    uart_putdecbyte(byte[0]);
    uart_putc('.');
    uart_putdecbyte(byte[1]);
    uart_putc('.');
    uart_putdecbyte(byte[2]);
    uart_putc('.');
    uart_putdecbyte(byte[3]);

} /* }}} */

#ifdef DEBUG
void uip_log(char *message)
/* {{{ */ {

    uart_puts_P("uip: ");

    while (*message != '\0')
        uart_putc(*message++);

    uart_eol();

} /* }}} */
#endif
