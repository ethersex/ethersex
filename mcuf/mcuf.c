/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Guido Pannenbecker <info@sd-gp.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
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
#include "../debug.h"
#include "../syslog/syslog.h"
#include "../clock/clock.h"
#include "mcuf.h"
#include "mcuf_net.h"
#include "mcuf_text.h"
#include "../uip/uip.h"

#ifdef MCUF_SUPPORT

#ifdef MCUF_SERIAL_SUPPORT
#define USE_USART MCUF_USE_USART
#define BAUD MCUF_BAUDRATE
#include "../usart.h"
#endif

/* MCUF_OUTPUT_SCREEN_... are now defined during Ethersex configuration
   and thus are defined in autoconf.h file.

   #define MCUF_OUTPUT_SCREEN_WIDTH 18
   #define MCUF_OUTPUT_SCREEN_HEIGHT 8 */

//TODO set MCUF_MAX_PCKT_SIZE correct
// 444,16,16 for 16x16borg
// 170/184,18,8 for blinkenledspro
#define MCUF_MAX_PCKT_SIZE MCUF_OUTPUT_SCREEN_WIDTH*MCUF_OUTPUT_SCREEN_HEIGHT + 40


struct {
  uint8_t len;
  uint8_t sent;
  uint8_t data[MCUF_MAX_PCKT_SIZE];
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

struct mcuf_scrolltext_struct mcuf_scrolltext_buffer;

uint8_t gdata[MCUF_OUTPUT_SCREEN_HEIGHT][MCUF_OUTPUT_SCREEN_WIDTH];

uint8_t blp_toc = 250;

void mcuf_senddata();
#ifdef MCUF_SERIAL_SUPPORT
void mcuf_serial_senddata();
void tx_start(uint8_t len);
#endif

void updateframe();

void draw_box (uint8_t startx, uint8_t starty, uint8_t lengthx, uint8_t lengthy,
               uint8_t outercolor, uint8_t innercolor);

void mcuf_scrolltext();

#ifdef BLP_SUPPORT
void blp_output();
void blp_clock();
void blp_strobe();
void blp_setspalte(uint8_t spalte, uint8_t status);
#endif

#ifdef MCUF_SERIAL_SUPPORT
/* We generate our own usart init module, for our usart port */
generate_usart_init()
#endif

void mcuf_init(void) {
#ifdef MCUF_SERIAL_SUPPORT
  /* Initialize the usart module */
  usart_init();
  /* Disable the receiver */
  usart(UCSR,B) &= ~_BV(usart(RXCIE));
  debug_printf("mcuf serial...\n");
#endif
  buffer.len = 1;
  buffer.sent = 0;
  snprintf_P(textbuff, 36, PSTR("Hi  I'm your ethersex           ;-)"));
  scrolltext(0,0xff,0,3);
}

void mcuf_newdata(void) {
  /* If we send a packet, drop the new packet */
  if (buffer.sent < buffer.len) return;

  blp_toc=250;

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
    uint16_t maxvalue = htons(pkt->maxval);

    if (maxvalue > 255) maxvalue = 255;
    uint8_t multiplier = 255 / maxvalue;

    if ( channels * height * width > MCUF_MAX_PCKT_SIZE - 12 ) {
#ifdef SYSLOG_SUPPORT
      syslog_sendf("Warning: skipped MCUF-Frame because of chan, height "
                   "or width to big (max pckt size: 444 inkl 12 byte header):"
                   " %d * %d * %d", channels, height, width);
#endif
      return;
    }

    /* init output-buffer */
    memset(buffer.data, 0, 12+(height*width*channels));

    /* write frame-data to output-buffer */
    uint8_t x, y;
    if (channels == 3) {
      /* Handle RGB-Frames */
      for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
        for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
          if (height > y && width > x) {
            uint8_t red   = pkt->data[(x + (y * width)) * channels + 0];
            uint8_t green = pkt->data[(x + (y * width)) * channels + 1];
            uint8_t blue  = pkt->data[(x + (y * width)) * channels + 2];	
            buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] =
                (red + green + blue) / 3 * multiplier;
          }
        }
      }
    } else {
      /* for non-RGB-Frames use only channel 1 and ignore all others */
      for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
        for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
          if (height > y && width > x) {
            buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] =
                pkt->data[(x + (y * width)) * channels + 0 ] * multiplier;
          }
        }
      }
    }
    /* init writing of output-buffer to uart */
    mcuf_senddata(); 
  } else


  /* EBLP Magic bytes - see
     https://wiki.blinkenarea.org/index.php/ExtendedBlinkenlightsProtocol */
  if ( strncmp(uip_appdata, "\xfe\xed\xbe\xef", 4) == 0) {
    /* input */
    struct eblp_packet *pkt = (struct eblp_packet *)uip_appdata;
    uint16_t height = htons(pkt->height);
    uint16_t width = htons(pkt->width);

#ifdef MCUF_SERIAL_WORKAROUND_FOR_BAD_MCUF_UDP_PACKETS
    /* scan packet to determine maxvalue (workaround for some stupid programms
       that don't send eblp according to
       https://wiki.blinkenarea.org/index.php/ExtendedBlinkenlightsProtocol
       but use 1 as maxvaule instead of 255) */
    uint16_t maxvalue = 1;	/* assume we do have a packet
                                   from such a stupid programm */
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
    memset(buffer.data, 0, 12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH));

    /* write frame-data to output-buffer */
    if (maxvalue == 1) {
      /* b/w */
      for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
        for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
          if ((height > y) && (width > x)) {
            if (pkt->data[x + (y * width)] > 0) {
              buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] = 255;
            }
          }
        }
      }
    } else {
      /* grayscaled */
      for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
        for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
          if ((height > y) && (width > x)) {
            buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] = pkt->data[x + (y * width)];
          }
        }
      }
    }

    /* init writing of output-buffer to uart */
    mcuf_senddata(); 
  } else


  /* BLP Magic bytes - see https://wiki.blinkenarea.org/index.php/BlinkenlightsProtocol */
  if ( strncmp(uip_appdata, "\xde\xad\xbe\xef", 4) == 0) {
    /* input */
    struct blp_packet *pkt = (struct blp_packet *)uip_appdata;
    uint16_t height = htons(pkt->height);
    uint16_t width = htons(pkt->width);

    /* init output-buffer */
    memset(buffer.data, 0, 12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH));

    /* write frame-data to output-buffer */
    uint8_t x, y;
    for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
      for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
        if ((height > y) && (width > x)) {
          if (pkt->data[x + (y * width)] > 0) {
            buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] = 255;
          }
        }
      }
    }
    /* init writing of output-buffer to uart */
    mcuf_senddata();
  }
}

void mcuf_senddata() {
#ifdef BLP_SUPPORT
  buffer.len = 12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH);
#ifndef MCUF_SERIAL_SUPPORT
  buffer.sent=buffer.len;
#endif
#endif
#ifdef MCUF_SERIAL_SUPPORT
  mcuf_serial_senddata();
#endif
}

#ifdef MCUF_SERIAL_SUPPORT
void mcuf_serial_senddata() {
  /* write mcuf-header for the shifter-device (4 byte magic, height, width, channels, maxval)
     see https://wiki.blinkenarea.org/index.php/MicroControllerUnitFrame
     and https://wiki.blinkenarea.org/index.php/Shifter */
  memcpy_P(buffer.data, PSTR("\x23\x54\x26\x66\x00\x08\x00\x12\x00\x01\x00\xff"), 12);

  /* send (MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH) bytes of data - the data has already been writen to the outputbuffer by mcuf_newdata */

  /* start to send the buffer to uart */
  tx_start(12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH));
}

void tx_start(uint8_t len) {
  buffer.len = len;
  buffer.sent = 1;
  /* Enable the tx interrupt and send the first character */
  usart(UCSR,B) |= _BV(usart(TXCIE));
  usart(UDR) = buffer.data[0];
}

SIGNAL(usart(USART,_TX_vect)) {
  if (buffer.sent < buffer.len) {
    usart(UDR) = buffer.data[buffer.sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
  }
}
#endif /* MCUF_SERIAL_SUPPORT */

void mcuf_periodic(void) {
  static uint8_t blp_tic=0;
  blp_tic++;
  if (buffer.sent <= buffer.len) {
    blp_tic=0;
  }

  if ( mcuf_scrolltext_buffer.end != 0) {
    mcuf_scrolltext();
  }

  if (blp_tic > blp_toc) {
    blp_toc=10;
    // scroll to the left
    for (uint8_t i = 12; i < (12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH)); i+=MCUF_OUTPUT_SCREEN_WIDTH) {
      buffer.data[i+MCUF_OUTPUT_SCREEN_WIDTH]=buffer.data[i+MCUF_OUTPUT_SCREEN_WIDTH]-buffer.data[i];
      buffer.data[i]=buffer.data[i]+buffer.data[i+MCUF_OUTPUT_SCREEN_WIDTH];
      buffer.data[i+MCUF_OUTPUT_SCREEN_WIDTH]=-buffer.data[i+MCUF_OUTPUT_SCREEN_WIDTH]+buffer.data[i];
      for (uint8_t j = i; j < (i+MCUF_OUTPUT_SCREEN_WIDTH); j++) {
        buffer.data[j+1]=buffer.data[j+1]-buffer.data[j];
        buffer.data[j]=buffer.data[j]+buffer.data[j+1];
        buffer.data[j+1]=-buffer.data[j+1]+buffer.data[j];
      }
    }
    mcuf_senddata();
  }
#       ifdef BLP_SUPPORT
  blp_output();
#       endif
  if (buffer.sent == buffer.len)
    buffer.sent=buffer.len+1;
}

void mcuf_show_clock() {
  /* reset mcuf counter */
  blp_toc=250;
  char *weekdays = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
  struct clock_datetime_t date;
  clock_current_localtime(&date);

  snprintf_P(textbuff, 36, PSTR("%s. %.2d.%.2d.20%.2d   %.2d:%.2d:%.2d \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t \t."),
             weekdays + date.dow * 4, date.day, date.month, date.year, date.hour, date.min, date.sec);

#ifdef SYSLOG_SUPPORT
      syslog_sendf("mcuf: textbuffer %s\n", textbuff);
#endif

  scrolltext(0,0xff,0,10);
  updateframe();
}

void updateframe() {

  /* init output-buffer */
  memset(buffer.data, 0, 12+(MCUF_OUTPUT_SCREEN_HEIGHT * MCUF_OUTPUT_SCREEN_WIDTH));

  /* write frame-data to output-buffer */
  uint8_t x, y;
  for (y = 0; y < MCUF_OUTPUT_SCREEN_HEIGHT; y++) {
    for (x = 0; x < MCUF_OUTPUT_SCREEN_WIDTH; x++) {
      if ((MCUF_OUTPUT_SCREEN_HEIGHT > y) && (MCUF_OUTPUT_SCREEN_WIDTH > x)) {
        if (gdata[y][x] > 0) {
          buffer.data[12 + (x + (y * MCUF_OUTPUT_SCREEN_WIDTH))] = 255;
        }
      }
    }
  }

  /* init writing of output-buffer to uart */
  mcuf_senddata();
}

void mcuf_show_string(char * x) {
  blp_toc=250;
  memcpy(textbuff,x,36);
  scrolltext(0,0xff,0,1);
  updateframe();
}

void draw_box (uint8_t startx, uint8_t starty, uint8_t lengthx, uint8_t lengthy,
               uint8_t outercolor, uint8_t innercolor) {
  uint8_t nunx,nuny;
  const uint8_t endx = startx+lengthx-1;
  const uint8_t endy = starty+lengthy-1;
  uint8_t color;
  if ((lengthx != 0) && (lengthy != 0)) {
    for (nunx = startx; nunx <= endx; nunx++) {
      for (nuny = starty; nuny <= endy; nuny++) {
        if ((nunx == startx) || (nunx == endx) || (nuny == starty) ||
          (nuny == endy)) {
          color = outercolor;
        } else {
          color = innercolor;
        }
        gdata[nuny][nunx] = color;
      }
    }
  }
}

void mcuf_scrolltext() {
  static uint8_t blp_tic=0;
  blp_tic++;
  /* return while someone does some mcuf output */
  if (blp_tic == mcuf_scrolltext_buffer.waittime) {
    uint8_t i=0;
    if (mcuf_scrolltext_buffer.tomove != 0) {
      mcuf_scrolltext_buffer.tomove --;
      mcuf_scrolltext_buffer.posshift++;
      draw_box(0, mcuf_scrolltext_buffer.posy, MCUF_OUTPUT_SCREEN_WIDTH, 8, mcuf_scrolltext_buffer.bcolor, mcuf_scrolltext_buffer.bcolor);
      mcuf_scrolltext_buffer.posx = MCUF_OUTPUT_SCREEN_WIDTH-1;
      i = 0;
      while (i < mcuf_scrolltext_buffer.end) {
        mcuf_scrolltext_buffer.posx += draw_char(textbuff[i], mcuf_scrolltext_buffer.posx+i-mcuf_scrolltext_buffer.posshift, mcuf_scrolltext_buffer.posy, mcuf_scrolltext_buffer.color, 0, 1);
        i++;
      }
      updateframe();
      blp_tic=0;
      //wdt_kick();
      //_delay_ms(mcuf_scrolltext_buffer.waittime);
    } else
      mcuf_scrolltext_buffer.end = 0;
      blp_tic=0;
  }
}

#ifdef BLP_SUPPORT
void blp_output(void) {
  uint8_t idx=0,col,halfframe,gscale=1;
  int8_t row;
  //TODO enable grayscale
  //for (idx=0;gscale<4;gscale++) {
  for (halfframe=1;halfframe<2;halfframe--) {
    for (row=7;row>=0;row--) {
      for (col=0;col<9;col++,idx++) {
        //blp_setspalte(col, (  buffer.data[12 + (row*18) + (col * 2 + halfframe)] > (gscale*64) ? 1:0  ));
        blp_setspalte(col, (  buffer.data[12 + (row*18) + (col * 2 + halfframe)] ));
        if (((idx+1)%9) == 0)
          blp_clock();
      }
    }
  }
  blp_strobe();
  //}
}

void blp_clock(void) {
  PIN_SET(BLP_CLK);
  PIN_CLEAR(BLP_CLK);
}

void blp_strobe(void) {
  PIN_SET(BLP_STR);
  PIN_CLEAR(BLP_STR);
}

void blp_setspalte(uint8_t spalte, uint8_t status){
  switch (spalte){
    case 0 :
      if(status > 0){
        PIN_SET(BLP_DA_A);
      } else {
        PIN_CLEAR(BLP_DA_A);
      }
      break;

    case 1 :
      if(status > 0){
        PIN_SET(BLP_DA_B);
      } else {
        PIN_CLEAR(BLP_DA_B);
      }
      break;

    case 2 :
      if(status > 0){
        PIN_SET(BLP_DA_C);
      } else {
        PIN_CLEAR(BLP_DA_C);
      }
      break;

    case 3 :
      if(status > 0){
        PIN_SET(BLP_DA_D);
      } else {
        PIN_CLEAR(BLP_DA_D);
      }
      break;

    case 4 :
      if(status > 0){
        PIN_SET(BLP_DA_E);
      } else {
        PIN_CLEAR(BLP_DA_E);
      }
      break;

    case 5 :
      if(status > 0){
        PIN_SET(BLP_DA_F);
      } else {
        PIN_CLEAR(BLP_DA_F);
      }
      break;

    case 6 :
      if(status > 0){
        PIN_SET(BLP_DA_G);
      } else {
        PIN_CLEAR(BLP_DA_G);
      }
      break;

    case 7 :
      if(status > 0){
        PIN_SET(BLP_DA_H);
      } else {
        PIN_CLEAR(BLP_DA_H);
      }
      break;

    case 8 :
      if(status > 0){
        PIN_SET(BLP_DA_I);
      } else {
        PIN_CLEAR(BLP_DA_I);
      }
      break;

    default :
      break;
  }
}
#endif /* BLP_SUPPORT */
#endif /* MCUF_SUPPORT */
