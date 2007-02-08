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
        PT_SPAWN(&state->pt, &state->datapt, ethcmd_readbytes(state, sizeof(struct ethcmd_msg_t)));

        /* check message type */
#ifdef DEBUG_ETHCMD
        uart_puts_P("cmd: subsystem: 0x");
        uart_puthexbyte(state->msg.raw.sys);
        uart_eol();
#endif

        if (state->msg.raw.sys == ETHCMD_SYS_VERSION &&
            state->msg.version.cmd == ETHCMD_VERSION_REQUEST) {

#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: version request\r\n");
#endif
            /* reply with version information */
            state->msg.version.cmd = ETHCMD_VERSION_REPLY;
            state->msg.version.major = ETHCMD_PROTOCOL_MAJOR;
            state->msg.version.minor = ETHCMD_PROTOCOL_MINOR;

        } else if (state->msg.raw.sys == ETHCMD_SYS_FS20 &&
            state->msg.fs20.cmd == ETHCMD_FS20_SEND) {

#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: sending fs20 command: ");
            uart_puthexbyte(HI8(NTOHS(state->msg.fs20.housecode)));
            uart_puthexbyte(LO8(NTOHS(state->msg.fs20.housecode)));
            uart_putc(' ');
            uart_puthexbyte(state->msg.fs20.address);
            uart_putc(' ');
            uart_puthexbyte(state->msg.fs20.command);
            uart_eol();
#endif

            fs20_send(NTOHS(state->msg.fs20.housecode), state->msg.fs20.address, state->msg.fs20.command);

            /* reply with status 0 */
            state->msg.response.sys = ETHCMD_SYS_RESPONSE;
            state->msg.response.old_sys = ETHCMD_SYS_FS20;
            state->msg.response.status = 0;

        } else {
#ifdef DEBUG_ETHCMD
            uart_puts_P("cmd: invalid request, aborting\r\n");
#endif
            PT_EXIT(&state->pt);
        }

        uip_send(state->buffer, sizeof(struct ethcmd_msg_t));

        while(1) {

            if (uip_rexmit()) {
                uip_send(state->buffer, sizeof(struct ethcmd_msg_t));
#ifdef DEBUG_ETHCMD
                uart_puts_P("cmd: retransmitting...\r\n");
#endif
            }

            if (uip_acked())
                break;

            PT_YIELD(&state->pt);
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
