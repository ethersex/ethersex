/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 }}} */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "../eeprom.h"
#include "../bit-macros.h"
#include "../config.h"
#include "../net/mcuf_net.h"
#include "../syslog/syslog.h"
#include "mcuf.h"
#include "../uip/uip.h"

#include "../pinning.c"

#ifdef MCUF_SUPPORT

#define USE_USART MCUF_USE_USART
#include "../usart.h"

struct {
  uint8_t len;
  uint8_t sent;
  uint8_t data[170];
} buffer;

struct mcuf_packet {
  uint16_t magic[2];
  uint16_t height;
  uint16_t width;
  uint16_t channels;
  uint16_t maxval;
  uint8_t data[];
};

struct eblp_packet {
  uint16_t magic[2];
  uint32_t framenumber;
  uint16_t width;
  uint16_t height;
  uint8_t data[];
};

struct blp_packet {
  uint16_t magic[2];
  uint32_t framenumber;
  uint16_t width;
  uint16_t height;
  uint8_t data[];
};

void mcuf_serial_senddata();
void tx_start(uint8_t len);


/* We generate our own usart init module, for our usart port */
generate_usart_init(MCUF_UART_UBRR)


void
mcuf_init(void)
{
    /* Initialize the usart module */
    usart_init();
    /* Disable the receiver */
    usart(UCSR,B) &= ~_BV(usart(RXCIE));

    buffer.len = 0;
    buffer.sent = 0;
}

void
mcuf_newdata(void) 
{
  /* If we send a packet, drop the new packet */
  if (buffer.sent < buffer.len) return;


  /* MCUF Magic bytes - see https://wiki.blinkenarea.org/index.php/MicroControllerUnitFrame */
  if ( strncmp(uip_appdata, "\x23\x54\x26\x66", 4) == 0) {
    /* input */
    struct mcuf_packet *pkt = (struct mcuf_packet *)uip_appdata;
    uint16_t height = htons(pkt->height);
    uint16_t width = htons(pkt->width);
    uint16_t channels = htons(pkt->channels);
    if (channels < 1) {
#ifdef SYSLOG_SUPPORT
      syslog_sendf("Warning: forced channels of MCUF-Frame to 1 (orig value: %d)", channels);
#endif
      channels = 1;
    }
    uint16_t maxval = htons(pkt->maxval);
    
    if ( maxval != 255 && !( maxval == 1 && channels == 1  ) ) {
#ifdef SYSLOG_SUPPORT
      syslog_sendf("Warning: skiped MCUF-Frame cause of wrong maxval: %d", maxval);
#endif
      return;
    }
    
    /* init output-buffer */
    memset(buffer.data, 0, 12+144);

    /* write frame-data to output-buffer */
    uint8_t x, y;
    if (channels == 3) {
      /* Handle RGB-Frames */
#ifdef SYSLOG_SUPPORT
      //syslog_send_P(PSTR("Notice: Using avg of 3-channels-mcuf-frame"));
#endif
      for (y = 0; y < 8; y++) {
        for (x = 0; x < 18; x++) {
          if (height > y && width > x) {
            uint8_t red   = pkt->data[(x + (y * width)) * channels + 0];
            uint8_t green = pkt->data[(x + (y * width)) * channels + 1];
            uint8_t blue  = pkt->data[(x + (y * width)) * channels + 2];
            buffer.data[12 + (x + (y * 18))] = (red + green + blue) / 3;
          }
        }
      }
    } else {
      /* for non-RGB-Frames use only channel 1 and ignore all others */
#ifdef SYSLOG_SUPPORT
      //syslog_sendf("Notice: Using Channel 1 of MCUF-Frame (channels in Frame: %d)", channels);
#endif
      for (y = 0; y < 8; y++) {
        for (x = 0; x < 18; x++) {
          if (height > y && width > x) {
            buffer.data[12 + (x + (y * 18))] = pkt->data[(x + (y * width)) * channels + 0 ];
          }
        }
      }
    }
    /* init writing of output-buffer to uart */
    mcuf_serial_senddata(); 
  } else


  /* EBLP Magic bytes - see https://wiki.blinkenarea.org/index.php/ExtendedBlinkenlightsProtocol */
  if ( strncmp(uip_appdata, "\xfe\xed\xbe\xef", 4) == 0) {
    /* input */
    struct eblp_packet *pkt = (struct eblp_packet *)uip_appdata;
    uint16_t height = htons(pkt->height);
    uint16_t width = htons(pkt->width);

#ifdef MCUF_SERIAL_WORKAROUND_FOR_BAD_MCUF_UDP_PACKETS
    /* scan packet to determine maxvalue (workaround for some stupid programms that dont send eblp according to https://wiki.blinkenarea.org/index.php/ExtendedBlinkenlightsProtocol but use 1 as maxvaule instead of 255) */
    uint16_t maxvalue = 1; //assume we do have a packet from such a stupid programm
    uint8_t x, y;
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        if (pkt->data[x + (y * width)] > 1) {
          maxvalue = 255; //everything is fine and as described in the doku - use 255 as maxvalue
          //syslog_sendf("Notice: found value bigger 1 on x %d y %d value %d", x, y, pkt->data[x + (y * width)]);
          goto workaround_break_label;
        }
      }
    }
    workaround_break_label:
#ifdef SYSLOG_SUPPORT
    if (maxvalue == 1) {
      //syslog_send_P(PSTR("Notice: Using maxvalue-workaround!"));
    }
#endif
#else
    uint16_t maxvalue = 255;
    uint8_t x, y;
#endif
    /* init output-buffer */
    memset(buffer.data, 0, 12+144);

    /* write frame-data to output-buffer */
    if (maxvalue == 1) {
      /* b/w */
      for (y = 0; y < 8; y++) {
        for (x = 0; x < 18; x++) {
          if ((height > y) && (width > x)) {
            if (pkt->data[x + (y * width)] > 0) {
              buffer.data[12 + (x + (y * 18))] = 255;
            }
          }
        }
      }
    } else {
      /* grayscaled */
      for (y = 0; y < 8; y++) {
        for (x = 0; x < 18; x++) {
          if ((height > y) && (width > x)) {
            buffer.data[12 + (x + (y * 18))] = pkt->data[x + (y * width)];
          }
        }
      }
    }

    /* init writing of output-buffer to uart */
    mcuf_serial_senddata(); 
  } else


  /* BLP Magic bytes - see https://wiki.blinkenarea.org/index.php/BlinkenlightsProtocol */
  if ( strncmp(uip_appdata, "\xde\xad\xbe\xef", 4) == 0) {
    /* input */
    struct blp_packet *pkt = (struct blp_packet *)uip_appdata;
    uint16_t height = htons(pkt->height);
    uint16_t width = htons(pkt->width);

    /* init output-buffer */
    memset(buffer.data, 0, 12+144);

    /* write frame-data to output-buffer */
    uint8_t x, y;
    for (y = 0; y < 8; y++) {
      for (x = 0; x < 18; x++) {
        if ((height > y) && (width > x)) {
          if (pkt->data[x + (y * width)] > 0) {
            buffer.data[12 + (x + (y * 18))] = 255;
          }
        }
      }
    }

    /* init writing of output-buffer to uart */
    mcuf_serial_senddata(); 
  }
}

void
mcuf_serial_senddata()
{
  /* write mcuf-header for the shifter-device (4 byte magic, height, width, channels, maxval)
     see https://wiki.blinkenarea.org/index.php/MicroControllerUnitFrame
     and https://wiki.blinkenarea.org/index.php/Shifter */
  memcpy_P(buffer.data, PSTR("\x23\x54\x26\x66\x00\x08\x00\x12\x00\x01\x00\xff"), 12);

  /* send 144 bytes of data - the data has already been writen to the outputbuffer by mcuf_newdata */

  /* start to send the buffer to uart */
  tx_start(12+144);
}

void
tx_start(uint8_t len)
{
    buffer.len = len;
    buffer.sent = 1;
    /* Enable the tx interrupt and send the first character */
    usart(UCSR,B) |= _BV(usart(TXCIE));
    usart(UDR) = buffer.data[0];
}

SIGNAL(usart(USART,_TX_vect))
{
  if (buffer.sent < buffer.len) {
    usart(UDR) = buffer.data[buffer.sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
  }
}
#endif
