/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "vnc.h"
#include "vnc_state.h"

#include "config.h"

uip_conn_t *vnc_conn = NULL;

char PROGMEM server_init[] = {
  0,0xff, // framebuffer-width
  0,0xff, // framebuffer-height

  // server-pixel-format
  0x08, // bits-per-pixel
  0x08, // depth
  0x01, // big-endian-ßag (ignored for 8-bit)
  0x01, // true-colour-ßag
  0x00, 0x07,// red-max (2 bit)
  0x00, 0x07,// green-max (2 bit)
  0x00, 0x03,// blue-max (2 bit)
  0x00, // red-shift -- assume bbgggrrr (bgr233)
  0x03, // green-shift
  0x06, // blue-shift 
  0x00, 0x00, 0x00, // padding

  0x00, 0x00, 0x00, 0x08, // name-length
  'E','t','h','e','r','s','e', 'x', // name-string
};

char PROGMEM server_name[] = "ethersex";

#define STATE (&uip_conn->appstate.vnc)

static void 
vnc_main(void)
{
    if (uip_aborted() || uip_timedout()) {
        VNCDEBUG ("connection aborted\n");
        vnc_conn = NULL;
    }

    if (uip_closed()) {
    VNCDEBUG ("connection closed\n");
        vnc_conn = NULL;
    }

    if (uip_connected()) {
    VNCDEBUG ("new connection\n");
        STATE->state = VNC_STATE_SEND_VERSION;
    }

    if (uip_acked() && STATE->state < VNC_STATE_IDLE)
      STATE->state++;

    if (uip_acked() 
        || (uip_poll() && STATE->state >= VNC_STATE_IDLE)
        || uip_rexmit() 
        || uip_connected() 
        || uip_newdata()) {
      VNCDEBUG("Send Data, state: %d\n", STATE->state);
      if (STATE->state == VNC_STATE_SEND_VERSION) {
        memcpy_P(uip_sappdata, PSTR("RFB 003.003\n"), 12);
        uip_send(uip_sappdata, 12);
      } else if ( STATE->state == VNC_STATE_SEND_AUTH) {
        ((char *)uip_appdata)[0] = 0;
        ((char *)uip_appdata)[1] = 0;
        ((char *)uip_appdata)[2] = 0;
        ((char *)uip_appdata)[3] = 1; /* VNC_AUTH_NONE */
        uip_send(uip_sappdata, 4);
      } else if ( STATE->state == VNC_STATE_SEND_CONFIG) {
        memcpy_P(uip_sappdata, server_init, sizeof(server_init));

        uip_send(uip_sappdata, sizeof(server_init)); 
        VNCDEBUG("sent %d bytes\n", sizeof(server_init)); 
        VNCDEBUG("first bytes: %02x %02x %02x %02x\n",
                ((char *)uip_sappdata)[0],
                ((char *)uip_sappdata)[1],
                ((char *)uip_sappdata)[2],
                ((char *)uip_sappdata)[3]);
      }
    }
}


void vnc_init(void)
{
  uip_listen(HTONS(VNC_PORT), vnc_main);
}

/*
  -- Ethersex META --
  header(services/vnc/vnc.h)
  net_init(vnc_init)
*/
