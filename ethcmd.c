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

#include <string.h>

#include "ethcmd.h"
#include "uip/uip.h"
#include "uart.h"
#include "onewire/onewire.h"

#define NTOHS(x) ((0x00ff & (x)) << 8 | (0xff00 & (x)) >> 8)

void ethcmd_init(void)
/* {{{ */ {

    uip_listen(HTONS(ETHCMD_PORT));

} /* }}} */

void ethcmd_main(void)
/* {{{ */ {

    if (uip_poll())
        return;

    //uart_puts_P("cmd: main(), uip_flags 0x");
    //uart_puthexbyte(uip_flags);
    //uart_eol();

    if (uip_aborted())
        uart_puts_P("cmd: connection aborted\r\n");

    if (uip_timedout())
        uart_puts_P("cmd: connection timed out\r\n");

    if (uip_closed())
        uart_puts_P("cmd: connection closed\r\n");

    if (uip_connected()) {
        uart_puts_P("cmd: new connection\r\n");
        //strcpy_P(uip_appdata, "foo!");
        //uip_send(uip_appdata, 4);
        uip_conn->appstate.ethcmd.foo = 0;
    }

    if (uip_acked() && uip_conn->appstate.ethcmd.foo == 1) {

        struct ethcmd_message_t *msg = (struct ethcmd_message_t *)uip_appdata;

        int ret = ow_search_rom_next();

        if (ret > 0) {
            uart_puts_P("cmd: sending next ow rom id\r\n");

            msg->length = HTONS(sizeof(struct ethcmd_message_t)
                    + sizeof(struct ethcmd_onewire_message_t));
            msg->message_type = HTONS(ETHCMD_MESSAGE_TYPE_ONEWIRE);

            struct ethcmd_onewire_message_t *ow_msg = msg->data;
            memcpy(&ow_msg->id, &ow_global.current_rom, 8);

            uip_send(uip_appdata, sizeof(struct ethcmd_message_t)
                    + sizeof(struct ethcmd_onewire_message_t));
        } else {
            uart_puts_P("cmd: last rom id sent, closing connection\r\n");
            uip_conn->appstate.ethcmd.foo = 0;
            uip_close();
            return;
        }

    }

    if (uip_newdata()) {

        struct ethcmd_message_t *msg = (struct ethcmd_message_t *)uip_appdata;

        uart_puts_P("cmd: data received, length 0x");
        uart_puthexbyte(HIGH(uip_len));
        uart_puthexbyte( LOW(uip_len));
        uart_eol();

        uart_puts_P("cmd: length: 0x");
        uart_puthexbyte( LOW(msg->length));
        uart_puthexbyte(HIGH(msg->length));
        uart_puts_P(", mesage_type: 0x");
        uart_puthexbyte( LOW(msg->message_type));
        uart_puthexbyte(HIGH(msg->message_type));
        uart_eol();

        if (msg->message_type == NTOHS(ETHCMD_MESSAGE_TYPE_ONEWIRE)) {
            uart_puts_P("cmd: detected onewire discover\r\n");

            /* new discover */
            uip_conn->appstate.ethcmd.foo = 1;

            int ret = ow_search_rom_first();

            if (ret > 0) {

                uart_puts_P("cmd: found ow rom\r\n");

                msg->length = HTONS(sizeof(struct ethcmd_message_t)
                                  + sizeof(struct ethcmd_onewire_message_t));
                msg->message_type = HTONS(ETHCMD_MESSAGE_TYPE_ONEWIRE);

                struct ethcmd_onewire_message_t *ow_msg = msg->data;
                memcpy(&ow_msg->id, &ow_global.current_rom, 8);

                uip_send(uip_appdata, sizeof(struct ethcmd_message_t)
                                    + sizeof(struct ethcmd_onewire_message_t));

            } else {

                uip_conn->appstate.ethcmd.foo = 0;

            }

        }

    }

} /* }}} */
