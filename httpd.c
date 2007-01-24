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
#include "uip/pt.h"
#include "uip/psock.h"
#include "httpd.h"

#ifdef DEBUG_HTTPD
#include "uart.h"
#endif

#define STATE (uip_conn->appstate.httpd)

char PROGMEM httpd_invalid_request[] =
/* {{{ */
"HTTP/1.1 400 Bad Request\n"
"Content-Length: 16\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n"
"\n"
"Invalid Request\n";
/* }}} */

/* local prototypes */
unsigned short psock_send_str_P(void *data);

void httpd_init(void)
/* {{{ */ {

    uip_listen(HTONS(HTTPD_PORT));

} /* }}} */

static PT_THREAD(httpd_handle(void))
/* {{{ */ {

    PSOCK_BEGIN(&STATE.in);

    PSOCK_READTO(&STATE.in, ' ');

    /* if command is not GET, send 400 */
    if (strncasecmp_P(STATE.buffer, PSTR("GET "), 4) != 0) {
        PSOCK_GENERATOR_SEND(&STATE.in, psock_send_str_P, httpd_invalid_request);
        PSOCK_CLOSE_EXIT(&STATE.in);
    }

#if 0
    PSOCK_SEND_STR(&STATE.in, "Hello. What is your name?\n");
    PSOCK_READTO(&STATE.in, '\n');
    strncpy(STATE.name, STATE.buffer, sizeof(STATE.name));
    PSOCK_SEND_STR(&STATE.in, "Hello ");
    PSOCK_SEND_STR(&STATE.in, STATE.name);
#endif
    PSOCK_CLOSE(&STATE.in);

    PSOCK_END(&STATE.in);

} /* }}} */

unsigned short psock_send_str_P(void *data)
/* {{{ */ {

    strncpy_P(uip_appdata, data, strlen_P(data));
    return strlen_P(data);

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
        /* initialize struct */
        STATE.state = HTTPD_STATE_IDLE;
        STATE.timeout = 0;

        /* initialize protosockets for in and out */
        PSOCK_INIT(&STATE.in, STATE.buffer, sizeof(STATE.buffer));
    }

    if (uip_newdata()) {
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: new data\r\n");
#endif
        //http_handle_input();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {

        if (!uip_poll()) {
            STATE.timeout = 0;
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: action\r\n");
#endif
        }
        httpd_handle();
    }

} /* }}} */
