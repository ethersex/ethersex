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

#include "fc.h"
#include "uart.h"

void fc_init(void)
/* {{{ */ {

    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);

    struct uip_udp_conn *c = uip_udp_new(&ip, 0);

    if (c != NULL) {
        uip_udp_bind(c, HTONS(FC_UDP_PORT));

        c->appstate.fc.transmit_state = 0;
    }

} /* }}} */

void fc_handle_conn(void)
/* {{{ */ {

    if (uip_newdata()) {

        uint8_t *buffer = (uint8_t *)uip_appdata;

        /* select 9 bit frame */
        _UCSRB_UART0 |= _BV(UCSZ02);

        /* set address bit */
        UCSR0B |= _BV(TXB80);

        /* write address */
        //uart_putc(*buffer);
        UDR0 = *buffer;
        //UDR0 = 0x88;

        /* clear address bit */
        _UCSRB_UART0 &= ~_BV(TXB80);

        /* write data */
        for (uint16_t i = 1; i < uip_len; i++) {
            uart_putc(buffer[i]);
        }

        /* disable 9 bit frame */
        _UCSRB_UART0 &= ~_BV(UCSZ02);

    }

} /* }}} */
