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
#include "httpd.h"

#ifdef DEBUG_HTTPD
#include "uart.h"
#endif

#define STATE (uip_conn->appstate.httpd)

static const char *document = "\n<html><head><title>index</title></head><body>" \
                 "eine Testseite! :)" \
                 "</body></html>";

/* local prototypes */
void httpd_send(void);
void httpd_parse(void);
void httpd_acked(void);

void httpd_init(void)
/* {{{ */ {

    uip_listen(HTONS(HTTPD_PORT));

} /* }}} */

void httpd_send(void)
/* {{{ */ {

#ifdef DEBUG_HTTPD
    uart_puts_P("httpd: send, state is 0x");
    uart_puthexbyte(STATE.state);
    uart_eol();
#endif

    if (STATE.state == HTTPD_STATE_SENDSTATUS) {

        if (STATE.response == HTTPD_RESPONSE_404) {
            uip_send("HTTP/1.1 404 NOT FOUND\n\n", 23);
        } else if (STATE.response == HTTPD_RESPONSE_200) {
            uip_send("HTTP/1.1 200 OK\n", 16);
        } else {
            uip_send("HTTP/1.1 400 BAD REQUEST\n\n", 25);
        }
    } else if (STATE.state == HTTPD_STATE_SENDDOCUMENT) {
        uip_send(STATE.document, strlen(STATE.document));
    }
#if 0
    } else if (STATE.state == HTTPD_STATE_INVALID_REQUEST) {
        uip_send("HTTP/1.0 404 Not Found\n", 23);
    }
#endif

} /* }}} */

void httpd_parse(void)
/* {{{ */ {

    /* search for GET/POST */
    if (STATE.state == HTTPD_STATE_WAIT) {

        char *ptr = (char *)uip_appdata;

        if (strncasecmp_P(uip_appdata, PSTR("GET "), 4) == 0) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: get request\r\n");
#endif
            //STATE.state = HTTPD_STATE_GET;
            ptr += 4;
#if 0
        } else if (strncasecmp_P(uip_appdata, PSTR("POST "), 5) == 0) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: post request\r\n");
#endif
            STATE.state = HTTPD_STATE_POST;
            ptr += 5;
#endif
        } else {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: invalid request\r\n");
#endif
            STATE.state = HTTPD_STATE_SENDSTATUS;
            STATE.response = HTTPD_RESPONSE_400;
            return;
        }

        /* extract url (search for trailing whitespace) */
        uint8_t url_len;

        for (url_len = 0; url_len < HTTPD_MAX_URL_LENGTH; url_len++) {
            if (ptr[url_len] == ' ')
                break;
        }

        STATE.state = HTTPD_STATE_SENDSTATUS;

        /* check for length exceeded */
        if (url_len == HTTPD_MAX_URL_LENGTH) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: url length exceeded\r\n");
#endif
            STATE.response = HTTPD_RESPONSE_400;
            return;
        }

        /* else we found the url, check for known urls */
        if (strncmp_P(ptr, PSTR("/"), url_len) == 0) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: found url /\r\n");
#endif
            STATE.response = HTTPD_RESPONSE_200;
            STATE.document = document;
        } else if (strncmp_P(ptr, PSTR("/test"), url_len) == 0) {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: found url /test\r\n");
#endif
            STATE.response = HTTPD_RESPONSE_200;
            STATE.document = document;
        } else {
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: unknown url\r\n");
#endif
            STATE.response = HTTPD_RESPONSE_404;
        }

        STATE.state = HTTPD_STATE_SENDSTATUS;

    }

} /* }}} */

void httpd_acked(void)
/* {{{ */ {

#ifdef DEBUG_HTTPD
    uart_puts_P("httpd: acked, state is 0x");
    uart_puthexbyte(STATE.state);
    uart_eol();
#endif

    if (STATE.state == HTTPD_STATE_SENDSTATUS) {
        if (STATE.response == HTTPD_RESPONSE_200) {
            STATE.state = HTTPD_STATE_SENDDOCUMENT;
        } else
            uip_close();
    } else if (STATE.state == HTTPD_STATE_SENDDOCUMENT) {
        uip_close();
    }

} /* }}} */

void httpd_main(void)
/* {{{ */ {

    if (uip_aborted())
        uart_puts_P("httpd: connection aborted\r\n");

    if (uip_timedout())
        uart_puts_P("httpd: connection aborted\r\n");

    if (uip_closed())
        uart_puts_P("httpd: connection closed\r\n");

    if (uip_connected()) {
        uart_puts_P("httpd: new connection\r\n");
        STATE.state = HTTPD_STATE_WAIT;
        STATE.response = HTTPD_RESPONSE_EMPTY;
        STATE.document = NULL;
    }

    if (uip_newdata())
        httpd_parse();

    if (uip_acked())
        httpd_acked();

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {
        httpd_send();
    }

} /* }}} */
