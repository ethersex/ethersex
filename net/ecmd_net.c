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

#include "ecmd_net.h"
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "../debug.h"
#include "../ecmd/ecmd.h"

#include <string.h>

#include "../config.h"
#ifdef ECMD_SUPPORT
#define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

/* module local prototypes */
void newdata(void);

void ecmd_net_init()
{
#ifndef TEENSY_SUPPORT
  /* Without teensy support we use tcp */
    uip_listen(HTONS(ECMD_NET_PORT), ecmd_net_main);
#else
  /* If teensy support is enabled we use udp */
    uip_ipaddr_t ip;
    uip_ipaddr_copy (&ip, all_ones_addr);

    uip_udp_conn_t *udp_echo_conn = uip_udp_new (&ip, 0, ecmd_net_main);

    if (!udp_echo_conn) 
      return; /* dammit. */

    uip_udp_bind (udp_echo_conn, HTONS (ECMD_NET_PORT));
#endif
}

#ifndef TEENSY_SUPPORT
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

        memcpy(state->inbuf, uip_appdata, cplen);
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


        /* parse command and write output to state->outbuf, reserving at least
         * one byte for the terminating \n */
        l = ecmd_parse_command(state->inbuf + skip,
                                    state->outbuf,
                                    ECMD_OUTPUTBUF_LENGTH-1);

#ifdef DEBUG_ECMD_NET
        debug_printf("parser returned %d\n", l);
#endif

        /* check if the parse has to be called again */
        if (l <= -10) {
#ifdef DEBUG_ECMD_NET
            debug_printf("parser needs to be called again\n");
#endif

            state->parse_again = 1;
            l = -l - 10;
        }

#ifdef DEBUG_ECMD_NET
        debug_printf("parser really returned %d\n", l);
#endif

        if (l > 0) {
            state->outbuf[l++] = '\n';
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
#endif /* no TEENSY_SUPPORT */

void ecmd_net_main(void)
{
#ifndef TEENSY_SUPPORT
    struct ecmd_connection_state_t *state = &uip_conn->appstate.ecmd;

    if (!uip_poll()) {
#ifdef DEBUG_ECMD_NET
        debug_printf("ecmd_net_main()\n");
#endif
    }

    /*
    if(uip_aborted()) {
        aborted();
    }

    if(uip_timedout()) {
        timedout();
    }

    if(uip_closed()) {
        closed();
    }
    */

    if(uip_connected()) {
#ifdef DEBUG_ECMD_NET
        debug_printf("new connection\n");
#endif
        state->in_len = 0;
        state->out_len = 0;
        state->parse_again = 0;
        state->close_requested = 0;
        memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
    }

    if(uip_acked()) {
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
            if (l <= -10) {
                state->parse_again = 1;
                l = -l - 10;
            } else
                state->parse_again = 0;

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
#else /* TEENSY _SUPPORT */
    if(uip_newdata()) {
      uip_udp_conn_t echo_conn;
      uip_ipaddr_copy(echo_conn.ripaddr, BUF->srcipaddr);
      echo_conn.rport = BUF->srcport;
      echo_conn.lport = HTONS(ECMD_NET_PORT);

      uip_udp_conn = &echo_conn;
      /* Add \0 to the data and remove \n from the data */
      ((char *)uip_appdata)[uip_datalen()] = 0;
      if ( ((char * )uip_appdata)[uip_datalen() - 1] == '\n')
        ((char *)uip_appdata)[uip_datalen() - 1] = 0;
      /* Parse the Data */
      uip_slen = ecmd_parse_command(uip_appdata, uip_appdata, 
                                    UIP_BUFSIZE - UIP_IPUDPH_LEN) + 1;
      
      ((char *)uip_appdata)[uip_slen - 1] = '\n';
      uip_process(UIP_UDP_SEND_CONN); 
      fill_llh_and_transmit();

      uip_slen = 0;
    }
#endif
}


#endif /* ECMD_SUPPORT */
