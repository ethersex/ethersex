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
#include "core/bit-macros.h"
#include "vnc.h"
#include "vnc_state.h"
#include "vnc_block_factory.h"

#include "config.h"

uip_conn_t *vnc_conn = NULL;

static char PROGMEM server_init[] = {
  HI8(VNC_SCREEN_WIDTH),  LO8(VNC_SCREEN_WIDTH),  // framebuffer-width  
  HI8(VNC_SCREEN_HEIGHT), LO8(VNC_SCREEN_HEIGHT), // framebuffer-height 

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

#define STATE (&vnc_conn->appstate.vnc)

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
        vnc_conn = uip_conn;
        STATE->state = VNC_STATE_SEND_VERSION;
    }

    if (uip_acked() && STATE->state < VNC_STATE_IDLE)
        STATE->state++;
    else if (uip_acked() && STATE->state == VNC_STATE_UPDATE) {
      uint8_t i = 0, x, y;
      while (STATE->updates_sent[i][0] != 0xff && i < VNC_UPDATES_SENT_LENGTH) {
        x = STATE->updates_sent[i][0];
        y = STATE->updates_sent[i][1];
        STATE->update_map[y][x / 8] &= ~_BV(x % 8);
        i++;
      }
    }

    if (uip_newdata() && STATE->state >= VNC_STATE_IDLE) {
        struct vnc_pointer_event *pointer;
        uint8_t block_x, block_y;
        switch(((char *)uip_appdata)[0]) {
        case VNC_POINTER_EVENT: 
          VNCDEBUG("pointer event\n");
          pointer = (struct vnc_pointer_event *) uip_appdata;
          block_x = HTONS(pointer->x) / VNC_BLOCK_WIDTH;
          block_y = HTONS(pointer->y) / VNC_BLOCK_HEIGHT;
          STATE->update_map[block_y][block_x / 8 ] |= _BV(block_x % 8);
          STATE->state = VNC_STATE_UPDATE;
          break;
        case VNC_SET_PIXEL_FORMAT: 
          VNCDEBUG("set pixel format, ignoring\n");
          break;
        case VNC_FB_UPDATE_REQ:
          VNCDEBUG("Framebuffer update requested\n");
          if (((char *)uip_appdata)[1] != 1) { 
            /* Only update on non incremental updates */
            STATE->state = VNC_STATE_UPDATE;
            uint8_t i, j;
            for (i = 0; i < VNC_BLOCK_ROWS; i++)
              for (j = 0; j < VNC_BLOCK_COL_BYTES; j++)
                STATE->update_map[i][j] = 0xff;
          }
          break;
        }
        
    }

    if (uip_acked() 
        || (uip_poll() && STATE->state >= VNC_STATE_IDLE)
        || uip_rexmit() 
        || uip_connected() 
        || uip_newdata()) {
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
        VNCDEBUG("server init, sent %d bytes\n", sizeof(server_init)); 
      } else if (STATE->state == VNC_STATE_UPDATE) {
        uint8_t updating_block_count = 
                (uip_mss() - 4 ) / sizeof(struct gui_block) ;
        /* VNCDEBUG("we are able to update %d blocks at once\n", 
                updating_block_count); */

        struct vnc_update_header *update = (struct vnc_update_header *) uip_sappdata;
        uint8_t block = 0;
        memset(STATE->updates_sent, 0xff, sizeof(STATE->updates_sent));
        uint8_t y, x;
        for (y = 0; y < VNC_BLOCK_ROWS; y++) {
          for (x = 0; x < VNC_BLOCK_COLS; x++) {
            if (STATE->update_map[y][x / 8] & _BV(x % 8)) {
              STATE->updates_sent[block][0] = x;
              STATE->updates_sent[block][1] = y;
              vnc_make_block(&update->blocks[block], x, y);
              block++;
              if (block == updating_block_count) 
                goto end_update_block_finder;
            }
          }
        }
end_update_block_finder:
        if (block != updating_block_count) {
          VNCDEBUG("no to be updated block found, update finished\n");
          STATE->state = VNC_STATE_IDLE;
        }
        update->type = 0;
        update->padding = 0;
        update->block_count = HTONS(block);
        uip_send(uip_sappdata, 
                 4 + block * sizeof(struct gui_block));
    }
  }
}


void vnc_init(void)
{
  uip_listen(HTONS(VNC_PORT), vnc_main);
}

void
vnc_periodic(void)
{
  if (vnc_conn && STATE->state == VNC_STATE_IDLE) {
    STATE->state = VNC_STATE_UPDATE;
    uint8_t i, j;
    for (i = 0; i < VNC_BLOCK_ROWS; i++)
      for (j = 0; j < VNC_BLOCK_COL_BYTES; j++)
        STATE->update_map[i][j] = 0xff;
  }
}

/*
  -- Ethersex META --
  header(services/vnc/vnc.h)
  net_init(vnc_init)
  timer(100, vnc_periodic())

  state_header(services/vnc/vnc_state.h)
  state_tcp(struct vnc_connection_state_t vnc)
*/
