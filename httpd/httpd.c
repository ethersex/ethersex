/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../pt/pt.h"
#include "../uip/psock.h"
#include "httpd.h"
#include "base64.h"
#include "../eeprom.h"
#include "../ecmd_parser/ecmd.h"
#include "../config.h"
#include "../vfs/vfs.h"

#ifdef DEBUG_HTTPD
# include "../debug.h"
# define printf        debug_printf
#else
# define printf(...)   ((void)0)
#endif

/* quickfix: include fs from etherrape.c */
extern fs_t fs;

char PROGMEM httpd_header_200[] =
"HTTP/1.1 200 OK\n"
"Connection: close\n";

char PROGMEM httpd_header_ct_css[] =
"Content-Type: text/css; charset=iso-8859-1\n";

char PROGMEM httpd_header_ct_html[] =
"Content-Type: text/html; charset=iso-8859-1\n";

char PROGMEM httpd_header_ct_xhtml[] =
"Content-Type: application/xhtml+xml; charset=iso-8859-1\n";

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
#endif /* ECMD_PARSER_SUPPORT */

char PROGMEM httpd_header_400[] =
/* {{{ */
"HTTP/1.1 400 Bad Request\n"
"Connection: close\n"
"Content-Type: text/plain; charset=iso-8859-1\n";
/* }}} */

char PROGMEM httpd_header_gzip[] =
/* {{{ */
"Content-Encoding: gzip\n";

#ifdef HTTPD_AUTH_SUPPORT
char PROGMEM httpd_header_401[] =
/* {{{ */
"HTTP/1.1 401 UNAUTHORIZED\n"
"Connection: close\n"
"WWW-Authenticate: Basic realm=\"Secure Area\"\n"
"Content-Type: text/plain; charset=iso-8859-1\n";
/*}}}*/

char PROGMEM httpd_body_401[] =
/*{{{*/
"Authentification required\n";
/*}}}*/
#endif /* HTTPD_AUTH_SUPPORT */

char PROGMEM httpd_body_400[] =
/*{{{*/
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
unsigned short send_length_if(void *data);
unsigned short send_sd_f(void *data);
unsigned short send_file_f(void *data);
unsigned short send_file_if(void *data);

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

    /* This buffer is used to hold the requested url */
    state->buffer[PSOCK_DATALEN(&state->in) - 1] = 0;
    state->tmp_buffer = malloc(strlen(state->buffer) + 1);
    if (! state->tmp_buffer) 
        PSOCK_CLOSE_EXIT(&state->in);
    strcpy(state->tmp_buffer, state->buffer);

#ifdef HTTPD_AUTH_SUPPORT

    /* Consume the get line */
    PSOCK_READTO(&state->in, '\n');

    do {
      PSOCK_READTO(&state->in, '\n');
      state->buffer[PSOCK_DATALEN(&state->in) - 1] = 0;
      if (strncmp_P(state->buffer, PSTR("Authorization"), 13) == 0) {
        char *p = strstr(state->buffer, "Basic ");
        if (!p) goto auth_failed;
        (&state->in)->readptr[-1] = 0;
        p = (char *)(&state->in)->readptr;
        while (*p != ' ') p--;

        base64_str_decode(p+1);
        if (strncmp_P(p + 1, PSTR(CONF_HTTPD_USERNAME ":"), 
                      strlen(CONF_HTTPD_USERNAME ":")) != 0)
            goto auth_failed;
        char passwd[sizeof(((struct eeprom_config_t *)0)->httpd_auth_password) + 1];
        eeprom_restore(httpd_auth_password, passwd, sizeof(passwd));
        if (strncmp(passwd, p + 1 + strlen(CONF_HTTPD_USERNAME ":"), 
                    sizeof(passwd)) != 0)
                goto auth_failed;
        goto auth_success;

      }
    } while (PSOCK_DATALEN(&state->in) > 2);

auth_failed:    
    /* Authentification is required */
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_401);
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
    PSOCK_GENERATOR_SEND(&state->in, send_length_P, httpd_body_401);
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_body_401);
    PSOCK_CLOSE_EXIT(&state->in);
auth_success:
#endif /* HTTPD_AUTH_SUPPORT */

    if (state->tmp_buffer[0] == 0)
        strncpy_P(state->name, PSTR(HTTPD_INDEX), sizeof(state->name));
#ifdef ECMD_PARSER_SUPPORT
    else if (strncmp_P(state->tmp_buffer, PSTR(ECMD_INDEX "?"), 
                  strlen_P(PSTR(ECMD_INDEX "?"))) == 0) {
      /* ecmd interface */
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_200_ecmd);

        char *ptr = state->tmp_buffer + strlen_P(PSTR(ECMD_INDEX "?"));
        char *insert = state->tmp_buffer;

        while (*ptr) {
          if (*ptr == '+')
            *insert = ' ';
          else if (*ptr == '%') {
            uint8_t tmp = ptr[3];
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
        insert[0] = 0;

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

#ifdef VFS_SUPPORT
    else {
        strncpy(state->name, state->tmp_buffer, sizeof(state->name));
        if (strlen(state->tmp_buffer) >= sizeof(state->name))
            state->name[sizeof(state->name)-1] = '\0';
    }

    printf ("fs: httpd: request for file \"%s\"\n", state->name);
#endif	/* VFS_SUPPORT */

    free(state->tmp_buffer);

#ifdef VFS_SUPPORT
    state->fd = vfs_open (state->name);
    if (state->fd) {
      state->len = vfs_size (state->fd);

      printf ("httpd: got it, that's nice, let's go serve it (%ld bytes)!\n", 
	      state->len);

      /* send headers */
      PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_200);
      if (state->name[0] == 'X')
        PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_ct_xhtml);
      else if (state->name[0] == 'S')
	PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_ct_css);
      else
	PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_ct_html);

      /* send content-length header */
      PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
      PSOCK_GENERATOR_SEND(&state->in, send_length_if, state);

      /* generate content */
      while(state->len)
	PSOCK_GENERATOR_SEND(&state->in, send_sd_f, state);

      printf("httpd: sd-data generation completed.\n");
      PSOCK_CLOSE_EXIT(&state->in);
    }
    
#endif

    /* Unable to handle the request.  Therefore just send a 404 reply. */

    /* send headers */
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_404);
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_header_length);
    PSOCK_GENERATOR_SEND(&state->in, send_length_P, httpd_body_404);

    /* send body text */
    PSOCK_GENERATOR_SEND(&state->in, send_str_P, httpd_body_404);

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


#ifdef VFS_SUPPORT
unsigned short
send_length_if (void *data)
{
    struct httpd_connection_state_t *state =
	(struct httpd_connection_state_t *) data;

    sprintf_P (uip_appdata, PSTR ("%ld\n\n"), state->len);
    return strlen (uip_appdata);
}
#endif	/* VFS_SUPPORT */


#ifdef VFS_SUPPORT
unsigned short send_sd_f(void *data)
{
    struct httpd_connection_state_t *state =
      (struct httpd_connection_state_t *)data;

    vfs_size_t len = vfs_read (state->fd, uip_appdata, uip_mss ());

    if (len <= 0) {
      state->len = 0;
      len = 0;
    }

    state->len -= len;

    return len;
}
#endif /* VFS_SUPPORT */



static inline void
httpd_cleanup (struct httpd_connection_state_t *state)
{
#ifdef VFS_SUPPORT
    if (state->fd) {
	printf("httpd: cleaning left-over vfs-handle at %p.\n", state->fd);

	vfs_close (state->fd);
	state->fd = NULL;
    }
#endif	/* VFS_SUPPORT */
}

void
httpd_main(void)
{

    struct httpd_connection_state_t *state = &uip_conn->appstate.httpd;

    if (uip_aborted() || uip_timedout()) {
	httpd_cleanup (state);
        printf ("httpd: connection aborted\n");
    }

    if (uip_closed()) {
	httpd_cleanup (state);
        state->state = HTTPD_STATE_CLOSED;
        printf ("httpd: connection closed\n");
    }

    if (uip_poll()) {
        state->timeout++;

        if (state->timeout == HTTPD_TIMEOUT) {
            printf ("httpd: timeout\n");
	    httpd_cleanup (state);
            uip_close();
        }
    }

    if (uip_connected()) {
        printf ("httpd: new connection\n");

        /* initialize struct */
        state->state = HTTPD_STATE_IDLE;
        state->timeout = 0;

        /* initialize protosockets for in and out */
        PSOCK_INIT(&state->in, (unsigned char *)state->buffer, sizeof(state->buffer));
    }

    if (uip_newdata()) {
        printf ("httpd: new data\n");
        //http_handle_input();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_connected() ||
       uip_poll()) {

        if (!uip_poll()) {
            state->timeout = 0;
            printf ("httpd: action\n");
        }
        httpd_handle(state);
    }

}
