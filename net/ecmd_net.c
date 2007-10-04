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

/* module local prototypes */
void newdata(void);

void ecmd_net_init()
{
    uip_listen(HTONS(ECMD_NET_PORT), ecmd_net_main);
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

        /* parse command and write output to state->outbuf, reserving at least
         * one byte for the terminating \n */
        l = ecmd_parse_command(state->inbuf,
                                    state->outbuf,
                                    ECMD_OUTPUTBUF_LENGTH-1);

#ifdef DEBUG_ECMD_NET
        debug_printf("parser returned %d\n", l);
#endif

        if (l > 0) {
            state->outbuf[l++] = '\n';
            state->out_len = l;
        }

        memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
        state->in_len = 0;
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
        memset(state->inbuf, 0, ECMD_INPUTBUF_LENGTH);
        PT_INIT(&state->thread);
    }

    if(uip_acked()) {
        state->out_len = 0;
    }

    if(uip_newdata()) {
        newdata();
    }

    if(uip_rexmit() ||
            uip_newdata() ||
            uip_acked() ||
            uip_connected() ||
            uip_poll()) {
        if (state->out_len > 0)
            uip_send(state->outbuf, state->out_len);
    }
}


#endif /* ECMD_SUPPORT */
