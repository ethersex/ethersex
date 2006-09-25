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

#include "sntp.h"
#include "clock.h"

#ifdef DEBUG_SNTP
#include "uart.h"
#endif

void sntp_prepare_request(uip_ipaddr_t *server)
/* {{{ */ {

    struct uip_udp_conn *c = uip_udp_new(server, HTONS(SNTP_UDP_PORT));

    if (c != NULL) {
        uip_udp_bind(c, HTONS(SNTP_UDP_PORT));
        c->appstate.sntp.state = SNTP_STATE_WAIT;
        c->appstate.sntp.timeout = 0;
#ifdef DEBUG_SNTP
        uart_puts_P("sntp: udp connection prepared\r\n");

        uart_puts_P("sntp: local port is 0x");
        uart_puthexbyte(LO8(c->lport));
        uart_puthexbyte(HI8(c->lport));
        uart_puts_P(", remote 0x");
        uart_puthexbyte(LO8(c->rport));
        uart_puthexbyte(HI8(c->rport));
        uart_eol();
#endif
    }

} /* }}} */

void sntp_send_request(void)
/* {{{ */ {

    if (uip_udp_conn->appstate.sntp.timeout == 0) {

#ifdef DEBUG_SNTP
        uart_puts_P("sntp: sending request\r\n");
#endif

        /* null everything */
        uint8_t *p = uip_appdata;

        for (uint8_t i = 0; i < sizeof(struct sntp_header_t); i++)
            *p++ = 0;

        /* set required fields */
        struct sntp_header_t *sntp_header = (struct sntp_header_t *)uip_appdata;

        sntp_header->version = SNTP_VERSION;
        sntp_header->mode = SNTP_MODE_CLIENT;

        /* send packet */
        uip_udp_send(sizeof(struct sntp_header_t));

        /* set timeout for retransmit */
        uip_udp_conn->appstate.sntp.timeout = SNTP_TIMEOUT;

    } else
        uip_udp_conn->appstate.sntp.timeout--;

} /* }}} */

void sntp_handle_conn(void)
/* {{{ */ {

    if (uip_poll()) {

        switch (uip_udp_conn->appstate.sntp.state) {
            case SNTP_STATE_WAIT: sntp_send_request();
                                  break;
            case SNTP_STATE_RESPONSE: uip_udp_remove(uip_udp_conn);
                                      break;
        }
    }

    if (uip_newdata()) {

        #ifdef DEBUG_SNTP
        uart_puts_P("sntp: newdata\r\n");
        #endif

        struct sntp_header_t *sntp = (struct sntp_header_t *)uip_appdata;

        if (sntp->mode == SNTP_MODE_SERVER) {
            #ifdef DEBUG_SNTP
            uart_puts_P("sntp: detected answer from server\r\n");
            #endif

            clock_set_time(NTOHL(sntp->transmit_timestamp.seconds));

            #ifdef DEBUG_SNTP
            uint8_t *p = (uint8_t *)&sntp->transmit_timestamp.seconds;

            uart_puts_P("sntp: timestamp: 0x");
            for (uint8_t i = 0; i < 4; i++)
                uart_puthexbyte(*p++);
            uart_eol();
            #endif

            uip_udp_conn->appstate.sntp.state = SNTP_STATE_RESPONSE;
        }
    }

} /* }}} */
