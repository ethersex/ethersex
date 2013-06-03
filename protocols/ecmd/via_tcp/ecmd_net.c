/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (C) 2007, 2008 Christian Dietrich <stettberger@dokucode.de>
 * Copyright (C) 2008 Stefan Siegl <stesie@brokenpipe.de>
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
 */

#include "config.h"

#include "ecmd_net.h"
#include "protocols/uip/uip.h"
#include "services/pam/pam_prototypes.h"
#include "core/debug.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

#include <string.h>

#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

/* module local prototypes */
void newdata(void);

void ecmd_net_init()
{
  /* Without teensy support we use tcp */
    uip_listen(HTONS(ECMD_TCP_PORT), ecmd_net_main);
}

void newdata(void)
{
    struct ecmd_connection_state_t *state = &uip_conn->appstate.ecmd;


    uint16_t diff = ECMD_INPUTBUF_LENGTH - state->in_len;
    if (diff > 0) {
        int cplen;

        if (uip_datalen() <= diff)
            cplen = uip_datalen();
        else
            cplen = diff;

        memcpy(state->inbuf + state->in_len, uip_appdata, cplen);
        state->in_len += cplen;

#ifdef DEBUG_ECMD_NET
        debug_printf("copied %d bytes\n", cplen);
#endif
    } else {
#ifdef DEBUG_ECMD_NET
        debug_printf("buffer full\n");
#endif
    }

    char *lf = memchr(state->inbuf, '\n', state->in_len);

    if (lf != NULL ||
        memchr(uip_appdata, '\n', uip_datalen()) != NULL) {

#ifdef DEBUG_ECMD_NET
        debug_printf("calling parser\n");
#endif

        if (lf)
            *lf = '\0';
        else
            state->inbuf[ECMD_INPUTBUF_LENGTH-1] = '\0';

        /* kill \r */
        int l;
        for (l = 0; l < ECMD_INPUTBUF_LENGTH; l++)
          if (state->inbuf[l] == '\r')
            state->inbuf[l] = '\0';

        /* if the first character is ! close the connection after the last
         * byte is sent
         */
        uint8_t skip = 0;
        if (state->inbuf[0] == '!') {
          skip = 1;
          state->close_requested = 1;
        }
      
#ifdef ECMD_PAM_SUPPORT
        if (state->pam_state == PAM_UNKOWN) {
          if (strncmp_P(state->inbuf + skip, PSTR("auth "), 5) != 0) { 
            /* No authentification request */
auth_required:
            state->out_len = sprintf_P(state->outbuf, 
                                       PSTR("authentification required\n"));
            memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
            state->in_len = 0;
            return;          
          } else {
            char *user = state->inbuf + skip + 5; /* "auth " */
            char *pass = strchr(user + 1,' ');
            if (! pass) goto auth_required;
            *pass = 0;
            do { pass++; } while (*pass == ' ');
            char *p = strchr(pass, ' ');
            if (p) 
              *p = 0;
            /* Do the Pam request, the pam request will cache username and
             * passwort if its necessary. */
            pam_auth(user, pass, &state->pam_state);

	    // send authentification successfull message
	    if (state->pam_state == PAM_SUCCESS) {
	      state->out_len = sprintf_P(state->outbuf, PSTR("authentification successful\n"));
	      uip_send(state->outbuf, state->out_len);
	    }

            if (p && p[1] != 0) { /* There ist something after the PAM request */
              memmove(state->inbuf, p+1, strlen(p+1) + 1);
              skip = 0;
              state->in_len = strlen(p+1);
              if (state->pam_state == PAM_PENDING)
                state->parse_again = 1;
            } else {
              state->in_len = 0;
              return;
            }
          }
        }
        if (state->pam_state == PAM_PENDING || state->pam_state == PAM_DENIED)
          return; /* Pam Subsystem promisses to change this state */
#endif

        /* parse command and write output to state->outbuf, reserving at least
         * one byte for the terminating \n */
        l = ecmd_parse_command(state->inbuf + skip,
                                    state->outbuf,
                                    ECMD_OUTPUTBUF_LENGTH-1);

#ifdef DEBUG_ECMD_NET
        debug_printf("parser returned %d\n", l);
#endif

        /* check if the parse has to be called again */
        if (is_ECMD_AGAIN(l)) {
#ifdef DEBUG_ECMD_NET
            debug_printf("parser needs to be called again\n");
#endif

            state->parse_again = 1;
            l = ECMD_AGAIN(l);
        }

#ifdef DEBUG_ECMD_NET
        debug_printf("parser really returned %d\n", l);
#endif

        if (l > 0) {
            if (state->outbuf[l] != ECMD_NO_NEWLINE) state->outbuf[l++] = '\n';
            state->out_len = l;
        }

        if (!state->parse_again) {
#ifdef DEBUG_ECMD_NET
            debug_printf("clearing buffer\n");
#endif
            memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
            state->in_len = 0;
        }
    }
}

void ecmd_net_main(void)
{
    struct ecmd_connection_state_t *state = &uip_conn->appstate.ecmd;

    if (!uip_poll()) {
#ifdef DEBUG_ECMD_NET
        debug_printf("ecmd_net_main()\n");
#endif
    }

    if(uip_connected()) {
#ifdef DEBUG_ECMD_NET
        debug_printf("new connection\n");
#endif
        state->in_len = 0;
        state->out_len = 0;
        state->parse_again = 0;
        state->close_requested = 0;
#ifdef ECMD_PAM_SUPPORT
        state->pam_state = PAM_UNKOWN;
#endif
        memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
    }

#ifdef ECMD_PAM_SUPPORT
        if (state->pam_state == PAM_DENIED) {
          state->out_len = sprintf_P(state->outbuf, 
                                     PSTR("authentification failed\n"));
          state->close_requested = 1;
        }
#endif


    if(uip_acked()
#ifdef ECMD_PAM_SUPPORT
        || (state->pam_state == PAM_SUCCESS && state->in_len) 
#endif
       
       ) {
        state->out_len = 0;

        if (state->parse_again) {
#ifdef DEBUG_ECMD_NET
            debug_printf("transmission done, calling parser again\n");
#endif
            /* if the first character is ! close the connection after the last
             * byte is sent
             */
            uint8_t skip = 0;
            if (state->inbuf[0] == '!') {
              skip = 1;
              state->close_requested = 1;
            }

            /* parse command and write output to state->outbuf, reserving at least
             * one byte for the terminating \n */
            int l = ecmd_parse_command(state->inbuf + skip,
                    state->outbuf,
                    ECMD_OUTPUTBUF_LENGTH-1);

            /* check if the parse has to be called again */
            if (is_ECMD_AGAIN(l)) {
                state->parse_again = 1;
                l = ECMD_AGAIN(l);
            } else {
                state->parse_again = 0;
                /* We have to clear the input buffer */
                state->in_len = 0;
            }

            if (l > 0) {
                state->outbuf[l++] = '\n';
                state->out_len = l;
            }
        }
    }

    if(uip_newdata()) {
        newdata();
    }

    if(uip_rexmit() ||
            uip_newdata() ||
            uip_acked() ||
            uip_connected() ||
            uip_poll()) {
        if (state->out_len > 0) {
#ifdef DEBUG_ECMD_NET
            debug_printf("sending %d bytes\n", state->out_len);
#endif
            uip_send(state->outbuf, state->out_len);
        } else if (state->close_requested)
          uip_close();
    }
}

/*
  -- Ethersex META --
  header(protocols/ecmd/via_tcp/ecmd_net.h)
  net_init(ecmd_net_init)

  state_header(protocols/ecmd/via_tcp/ecmd_state.h)
  state_tcp(struct ecmd_connection_state_t ecmd)
*/
