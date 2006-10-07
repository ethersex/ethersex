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

#include "ethcmd.h"
#include "uip/uip.h"
#include "uart.h"

void ethcmd_init(void)
/* {{{ */ {

    uip_listen(HTONS(ETHCMD_PORT));

} /* }}} */

void ethcmd_parse_message(struct ethcmd_message_t *msg)
/* {{{ */ {

    switch (msg->message_type) {

        case HTONS(ETHCMD_MESSAGE_TYPE_VERSION):
            #ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: version request\r\n");
            #endif

            /* send version reply */
            struct ethcmd_message_version_t *m = (struct ethcmd_message_version_t *)msg;
            m->length = HTONS(sizeof(struct ethcmd_message_version_t) - 2);
            m->version = ETHCMD_PROTOCOL_VERSION;
            uip_send(msg, sizeof(struct ethcmd_message_version_t));

            break;

        default:
            #ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: unknown message type: 0x");
            uart_puthexbyte( LOW(msg->message_type));
            uart_puthexbyte(HIGH(msg->message_type));
            uart_eol();
            #endif
            break;

    }

} /* }}} */

void ethcmd_send(void)
/* {{{ */ {

    switch (uip_conn->appstate.ethcmd.state) {

        case ETHCMD_STATE_IDLE:
            #ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: sending version reply\r\n");
            #endif

            /* send version reply */
            struct ethcmd_message_version_t *m = (struct ethcmd_message_version_t *)uip_appdata;

            m->length = HTONS(sizeof(struct ethcmd_message_version_t) - 2);
            m->message_type = HTONS(ETHCMD_MESSAGE_TYPE_VERSION);
            m->version = ETHCMD_PROTOCOL_VERSION;

            uip_send(uip_appdata, sizeof(struct ethcmd_message_version_t));
            break;
    }

} /* }}} */

void ethcmd_main(void)
/* {{{ */ {

    if (uip_poll())
        return;

    #ifdef DEBUG_ETHCMD
    if (uip_aborted())
        uart_puts_P("cmd: connection aborted\r\n");

    if (uip_timedout())
        uart_puts_P("cmd: connection timed out\r\n");

    if (uip_closed())
        uart_puts_P("cmd: connection closed\r\n");
    #endif

    if (uip_aborted() ||
        uip_timedout() ||
        uip_closed())
            uip_conn->appstate.ethcmd.state = ETHCMD_STATE_DISCONNECTED;

    if (uip_connected()) {
        #ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: new connection\r\n");
        #endif

        uip_conn->appstate.ethcmd.state = ETHCMD_STATE_IDLE;
    }

    if (uip_newdata()) {

        struct ethcmd_message_t *msg = (struct ethcmd_message_t *)uip_appdata;

        #ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: data received, length 0x");
        uart_puthexbyte(HIGH(uip_len));
        uart_puthexbyte( LOW(uip_len));
        uart_eol();

        uart_puts_P("cmd: length: 0x");
        uart_puthexbyte( LOW(msg->length));
        uart_puthexbyte(HIGH(msg->length));
        uart_puts_P(", message_type: 0x");
        uart_puthexbyte( LOW(msg->length));
        uart_puthexbyte(HIGH(msg->length));
        uart_eol();
        #endif

        ethcmd_parse_message(msg);

    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {
        ethcmd_send();
    }

} /* }}} */
