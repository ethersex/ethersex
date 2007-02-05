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
#include "pt/pt.h"
#include "uart.h"
#include "onewire/onewire.h"
#include "fs20.h"

#define NTOHS(x) ((0x00ff & (x)) << 8 | (0xff00 & (x)) >> 8)

void ethcmd_init(void)
/* {{{ */ {

    uip_listen(HTONS(ETHCMD_PORT));

} /* }}} */

static PT_THREAD(ethcmd_readbytes(struct ethcmd_connection_state_t *state, uint16_t count))
/* {{{ */ {

    PT_BEGIN(&state->datapt);

#ifdef DEBUG_ETHCMD
    uart_puts_P("cmd: reading 0x");
    uart_puthexbyte(HI8(count));
    uart_puthexbyte(LO8(count));
    uart_puts_P(" bytes\r\n");
#endif

    while(1) {
        PT_WAIT_UNTIL(&state->datapt, uip_newdata());

        if (uip_datalen()+state->fill < count) {
            memcpy(state->buffer+state->fill, uip_appdata, uip_datalen());
            state->fill += uip_datalen();
            PT_YIELD(&state->datapt);
        } else {
            memcpy(state->buffer+state->fill, uip_appdata, (count-state->fill));
            state->data_length = uip_datalen()-count+state->fill;
            state->fill = count;
            memmove((uint8_t *)uip_appdata,
                    (uint8_t *)uip_appdata+count,
                    state->data_length);
            break;
        }
    }

    PT_EXIT(&state->datapt);
    PT_END(&state->datapt);

} /* }}} */

static PT_THREAD(ethcmd_handle(struct ethcmd_connection_state_t *state))
/* {{{ */ {

    PT_BEGIN(&state->pt);

    while(1) {
#ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: waiting for message\r\n");
#endif

        /* wait until enough bytes have been received */
        state->fill = 0;
        PT_SPAWN(&state->pt, &state->datapt, ethcmd_readbytes(state, sizeof(struct ethcmd_message_t)));

        /* check message type */
#ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: subsystem: 0x");
        uart_puthexbyte(LO8(state->msg.subsystem));
        uart_puthexbyte(HI8(state->msg.subsystem));
        uart_puts_P(", length: 0x");
        uart_puthexbyte(LO8(state->msg.length));
        uart_puthexbyte(HI8(state->msg.length));
        uart_puts_P(", still in uip_appdata: 0x");
        uart_puthexbyte(HI8(state->data_length));
        uart_puthexbyte(LO8(state->data_length));
        uart_eol();

        //uip_close();
#endif

        /* version request */
        if (state->msg.subsystem == HTONS(ETHCMD_MESSAGE_TYPE_VERSION)) {
#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: sending version reply\r\n");
#endif
            state->msg.data[0] = ETHCMD_SEND_VERSION;
            state->msg.data[1] = ETHCMD_PROTOCOL_MAJOR;
            state->msg.data[2] = ETHCMD_PROTOCOL_MINOR;
            state->msg.length = HTONS(sizeof(struct ethcmd_message_t)+3);

#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: sending bytes: 0x");
            uart_puthexbyte(sizeof(struct ethcmd_message_t)+3);
            uart_eol();
#endif

            uip_send(state->buffer, sizeof(struct ethcmd_message_t)+3);

            while(1) {

                if (uip_rexmit()) {
                    uip_send(state->buffer, sizeof(struct ethcmd_message_t)+3);
#ifdef DEBUG_ETHCMD
                    uart_puts_P("cmd: retransmitting...\r\n");
#endif
                }

                if (uip_acked())
                    break;

                PT_YIELD(&state->pt);
            }

#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: done sending version request\r\n");
#endif

        } else if (state->msg.subsystem == HTONS(ETHCMD_MESSAGE_TYPE_FS20)) {
#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: fs20 request\r\n");
#endif

            /* convert length */
            state->msg.length = HTONS(state->msg.length);

            if (state->msg.length >= sizeof(struct ethcmd_message_t)+
                                     sizeof(struct ethcmd_fs20_message_t)) {
                struct ethcmd_fs20_message_t *m = uip_appdata;
#ifdef DEBUG_ETHCMD
                uart_puts_P("cmd: command 0x");
                uart_puthexbyte(m->command);
                uart_putc(' ');
                uart_puthexbyte(LO8(m->fs20_housecode));
                uart_puthexbyte(HI8(m->fs20_housecode));
                uart_putc(' ');
                uart_puthexbyte(m->fs20_address);
                uart_putc(' ');
                uart_puthexbyte(m->fs20_command);
                uart_eol();
#endif

                if (m->command == ETHCMD_FS20_SEND) {
#ifdef DEBUG_ETHCMD
                    uart_puts_P("cmd: sending fs20 command\r\n");
#endif
                    fs20_send(NTOHS(m->fs20_housecode), m->fs20_address, m->fs20_command);
                }

            }

        }



        PT_YIELD(&state->pt);
    }

    PT_END(&state->pt);

} /* }}} */

void ethcmd_main(void)
/* {{{ */ {

    struct ethcmd_connection_state_t *state = &uip_conn->appstate.ethcmd;

#ifdef DEBUG_ETHCMD
    if (uip_aborted())
        uart_puts_P("cmd: connection aborted\r\n");
#endif

#ifdef DEBUG_ETHCMD
    if (uip_timedout())
        uart_puts_P("cmd: connection timed out\r\n");
#endif

#ifdef DEBUG_ETHCMD
    if (uip_closed())
        uart_puts_P("cmd: connection closed\r\n");
#endif

    if (uip_poll()) {
        state->timeout++;

        if (state->timeout == ETHCMD_TIMEOUT) {
#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: timeout\r\n");
#endif
            uip_close();
        }
    }

    if (uip_connected()) {
#ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: new connection\r\n");
#endif
        /* initialize struct */
        state->timeout = 0;

        /* init thread */
        PT_INIT(&state->pt);
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {

        if (!uip_poll())
            state->timeout = 0;

        /* call thread, close connection if the thread has exited */
        if (PT_SCHEDULE(ethcmd_handle(state)) == 0) {
#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: thread has exited, closing connection\r\n");
#endif
            uip_close();
        }
    }

} /* }}} */
