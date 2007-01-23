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
// #include "uip/psock.h"
#include "httpd.h"

#ifdef DEBUG_HTTPD
#include "uart.h"
#endif

#define STATE (uip_conn->appstate.httpd)

/* local prototypes */

void httpd_init(void)
/* {{{ */ {

    uip_listen(HTONS(HTTPD_PORT));

} /* }}} */

void httpd_main(void)
/* {{{ */ {

    if (uip_aborted())
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection aborted\r\n");
#endif

    if (uip_timedout())
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection aborted\r\n");
#endif

    if (uip_closed()) {
        STATE.state = HTTPD_STATE_CLOSED;
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection closed\r\n");
#endif
    }

    if (uip_poll()) {
        STATE.timeout++;

        if (STATE.timeout == HTTPD_TIMEOUT) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: timeout\r\n");
#endif
            uip_close();
        }
    }


    if (uip_connected()) {
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: new connection\r\n");
#endif
        STATE.state = HTTPD_STATE_IDLE;
        STATE.timeout = 0;
    }

    if (uip_newdata()) {
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: new data\r\n");
#endif
        //http_handle_input();
    }

#if 0
    if (uip_acked())
        httpd_acked();

#endif
    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {

#ifdef DEBUG_HTTPD
        if (!uip_poll()) {
            uart_puts_P("httpd: action\r\n");
        }
#endif
    }

} /* }}} */
