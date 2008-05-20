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

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../pt/pt.h"
#include "../uip/psock.h"
#include "httpd.h"
#include "../dataflash/fs.h"
#include "../ecmd_parser/ecmd.h"

#ifdef DEBUG_HTTPD
#include "uart.h"
#endif

/* quickfix: include fs from etherrape.c */
extern fs_t fs;

char PROGMEM httpd_header_200[] =
/* {{{ */
"HTTP/1.1 200 OK\n"
"Connection: close\n"
"Content-Type: text/html; charset=iso-8859-1\n";
/* }}} */

#ifdef ECMD_PARSER_SUPPORT
char PROGMEM httpd_header_200_ecmd[] =
/* {{{ */
/* Please note: this is the _whole_ header, no content-length must follow */
"HTTP/1.1 200 OK\n"
"Connection: close\n"
"Cache-Control: no-cache\n"
"Cache-Control: must-revalidate\n"
"Content-Type: text/plain; charset=iso-8859-1\n\n";
/* }}} */
#endif

char PROGMEM httpd_header_400[] =
/* {{{ */
"HTTP/1.1 400 Bad Request\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n";

char PROGMEM httpd_body_400[] =
"Bad Request\n";
/* }}} */

char PROGMEM httpd_header_404[] =
/* {{{ */
"HTTP/1.1 404 File Not Found\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n";

char PROGMEM httpd_body_404[] =
"File Not Found\n";
/* }}} */

char PROGMEM httpd_header_length[] = "Content-Length: ";

/* local prototypes */
unsigned short send_str_with_nl(void *data);
unsigned short send_str_P(void *data);
unsigned short send_length_P(void *data);
unsigned short send_length_f(void *data);
unsigned short send_file_f(void *data);

void httpd_init(void)
/* {{{ */ {

    uip_listen(HTONS(HTTPD_PORT), httpd_main);
    uip_listen(HTONS(HTTPD_ALTERNATE_PORT), httpd_main);

} /* }}} */

static PT_THREAD(httpd_handle(struct httpd_connection_state_t *state))
/* {{{ */ {

    PSOCK_BEGIN(&state->in);

    PSOCK_READTO(&state->in, ' ');

    /* if command is not GET, send 400 */
    if (!(strncasecmp_P(state->buffer, PSTR("GET "), 4) == 0)) {
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_400);
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
        PSOCK_GENERATOR_SEND(&state->in, send_length_P, httpd_body_400);
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_body_400);
        PSOCK_CLOSE_EXIT(&state->in);
    }

    /* else search for file */
    PSOCK_READTO(&state->in, '/');
    PSOCK_READTO(&state->in, ' ');

    if (state->buffer[0] == ' ')
        strncpy_P(state->name, PSTR(HTTPD_INDEX), sizeof(state->name));
#ifdef ECMD_PARSER_SUPPORT
    else if (strncmp_P(state->buffer, PSTR(ECMD_INDEX "?"), 
                  strlen_P(PSTR(ECMD_INDEX "?"))) == 0) {
      /* ecmd interface */
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_200_ecmd);

        char *ptr = state->buffer + strlen_P(PSTR(ECMD_INDEX "?"));
        /* This buffer is used to save the command buffer, because we have to
         * call ecmd_parse_command more than once, e.g. with `fs list' 
         */
        state->tmp_buffer =
          malloc(strlen(state->buffer) - strlen_P(PSTR(ECMD_INDEX "?")));

        if (! state->tmp_buffer) 
          PSOCK_CLOSE_EXIT(&state->in);
        char *insert = state->tmp_buffer;

        while (*ptr) {
          if (*ptr == '+')
            *insert = ' ';
          else if (*ptr == '%') {
            uint8_t data, tmp = ptr[3];
            ptr[3] = 0;
            *insert = strtol(ptr + 1, NULL, 16);
            ptr[3] = tmp;
            ptr += 2;
          } else {
            *insert = *ptr;
          }
          insert++;
          ptr++;
        }
        insert[-1] = 0;

        /* the ecmd parser call, this call maybe repeated */
        int16_t len;
        do {
          len = ecmd_parse_command(state->tmp_buffer, state->buffer, 
                                   sizeof(state->buffer)-1);
          uint16_t real_len;
          if (len <= -10) {
            real_len = -len - 10;
            state->parse_again = 1;
          } else {
            real_len = len;
            state->parse_again = 0;
          }
          state->buffer[real_len] = 0;

          PSOCK_GENERATOR_SEND(&state->in, send_str_with_nl, state->buffer);
        } while (state->parse_again);

        /* free the temporary command buffer */
        free(state->tmp_buffer);

        PSOCK_CLOSE_EXIT(&state->in);
    }
#endif
    else {
        strncpy(state->name, state->buffer, sizeof(state->name));
        if (PSOCK_DATALEN(&state->in) < sizeof(state->name))
            state->name[PSOCK_DATALEN(&state->in)-1] = '\0';
        else
            state->name[sizeof(state->name)-1] = '\0';
    }

#ifdef DEBUG_HTTPD
    uart_puts_P("fs: httpd: request for file \"");
    uart_puts(state->name);
    uart_puts_P("\"\r\n");
    uart_puts_P("fs: searching file inode: 0x");
#endif

    /* search inode */
    state->inode = fs_get_inode(&fs, state->name);

#ifdef DEBUG_HTTPD
    uart_puthexbyte(HI8(state->inode));
    uart_puthexbyte(LO8(state->inode));
    uart_eol();
#endif

    if (state->inode == 0xffff) {
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: file not found, sending 404\r\n");
#endif

        /* send headers */
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_404);
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
        PSOCK_GENERATOR_SEND(&state->in, send_length_P, httpd_body_404);

        /* send body text */
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_body_404);
        PSOCK_CLOSE_EXIT(&state->in);
    } else {
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: file found\r\n");
#endif

        /* reset offset */
        state->offset = 0;

        /* send headers */
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_200);
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
        PSOCK_GENERATOR_SEND(&state->in, send_length_f, &state->inode);

        /* send file */
        while(state->inode != 0xffff) {
            PSOCK_GENERATOR_SEND(&state->in, send_file_f, state);
        }
    }

#if 0
    PSOCK_SEND_STR(&state->in, "Hello. What is your name?\n");
    PSOCK_READTO(&state->in, '\n');
    strncpy(state->name, state->buffer, sizeof(state->name));
    PSOCK_SEND_STR(&state->in, "Hello ");
    PSOCK_SEND_STR(&state->in, state->name);
#endif
    PSOCK_CLOSE(&state->in);

    PSOCK_END(&state->in);

} /* }}} */

unsigned short send_str_with_nl(void *data)
/* {{{ */ {

    sprintf_P(uip_appdata, PSTR("%s\n"), data);
    return strlen(uip_appdata);

} /* }}} */


unsigned short send_str_P(void *data)
/* {{{ */ {

    strncpy_P(uip_appdata, data, strlen_P(data));
    return strlen_P(data);

} /* }}} */

unsigned short send_length_P(void *data)
/* {{{ */ {

    sprintf_P(uip_appdata, PSTR("%d\n\n"), strlen_P(data));
    return strlen(uip_appdata);

} /* }}} */

unsigned short send_length_f(void *data)
/* {{{ */ {

    fs_inode_t i = *((fs_inode_t *)data);

    sprintf_P(uip_appdata, PSTR("%d\n\n"), fs_size(&fs, i));
    return strlen(uip_appdata);

} /* }}} */

unsigned short send_file_f(void *data)
/* {{{ */ {

    struct httpd_connection_state_t *state = (struct httpd_connection_state_t *)data;
    fs_size_t len = fs_read(&fs, state->inode, uip_appdata, state->offset, uip_mss());

#ifdef DEBUG_HTTPD
    uart_puts_P("httpd: mss is 0x");
    uart_puthexbyte(HI8(uip_mss()));
    uart_puthexbyte(LO8(uip_mss()));
    uart_puts_P(", len is 0x");
    uart_puthexbyte(HI8(uip_mss()));
    uart_puthexbyte(LO8(uip_mss()));
    uart_eol();
#endif

    /* if this was all, reset state->inode */
    if (len < uip_mss())
        state->inode = 0xffff;
    else
        state->offset += len;

    return len;

} /* }}} */

void httpd_main(void)
/* {{{ */ {

    struct httpd_connection_state_t *state = &uip_conn->appstate.httpd;

    if (uip_aborted())
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection aborted\r\n");
#endif

    if (uip_timedout())
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection aborted\r\n");
#endif

    if (uip_closed()) {
        state->state = HTTPD_STATE_CLOSED;
#ifdef DEBUG_HTTPD
        uart_puts_P("httpd: connection closed\r\n");
#endif
    }

    if (uip_poll()) {
        state->timeout++;

        if (state->timeout == HTTPD_TIMEOUT) {
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
        state->state = HTTPD_STATE_IDLE;
        state->timeout = 0;

        /* initialize protosockets for in and out */
        PSOCK_INIT(&state->in, state->buffer, sizeof(state->buffer));
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
            state->timeout = 0;
#ifdef DEBUG_HTTPD
            uart_puts_P("httpd: action\r\n");
#endif
        }
        httpd_handle(state);
    }

} /* }}} */
