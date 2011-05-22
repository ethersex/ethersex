/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "dc3840.h"

#include "protocols/ecmd/ecmd-base.h"

/* USART cruft. */
#define USE_USART DC3840_USE_USART
#define BAUD 921600
#include "core/usart.h"

#ifndef noinline
#define noinline __attribute__((noinline))
#endif

/* Length of current image, declared extern in dc3840.h */
uint16_t dc3840_data_length;

/* We generate our own usart init module, for our usart port */
generate_usart_init()

/* Buffer the RX-vector stores the command reply to. */
static volatile uint8_t dc3840_reply_buf[16];

/* How many bytes have been received since last command. */
static volatile uint16_t dc3840_reply_ptr;

/* Where to store next captured byte. */
static volatile uint8_t *dc3840_capture_ptr;

/* From which byte on to start capturing.  Counted down in RX vector,
   capturing starts if it gets zero. */
static volatile uint16_t dc3840_capture_start;

/* How many bytes to capture.  Counted down in RX vector as well. */
static volatile uint16_t dc3840_capture_len;

/* Send one single byte to camera UART. */
static void noinline dc3840_send_uart (uint8_t byte);

/* Send a command to the camera and wait for ACK (return 0).
   This function automatically repeats the command up to three times.
   Returns 1 on timeout or NAK. */
static uint8_t noinline dc3840_send_command
	(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);


/* #define DC3840_UDP_DEBUG 1 */


static void
dc3840_send_uart (uint8_t byte)
{
  while (!(usart(UCSR,A) & _BV(usart(UDRE))));
  usart(UDR) = byte;
}


static uint8_t
dc3840_send_command (uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
{
  uint8_t retries = 8;
  DC3840_DEBUG ("-> %02x %02x %02x %02x %02x\n", a, b, c, d, e);

  do
    {
      dc3840_reply_ptr = 0;	/* Reset. */

      /* Send command sequence first */
      dc3840_send_uart (0xFF);
      dc3840_send_uart (0xFF);
      dc3840_send_uart (0xFF);

      /* Send payload. */
      dc3840_send_uart (a);
      dc3840_send_uart (b);
      dc3840_send_uart (c);
      dc3840_send_uart (d);
      dc3840_send_uart (e);

      if (a == DC3840_CMD_ACK || a == DC3840_CMD_NAK)
	return 0;		/* ACK */

      uint8_t timeout = 100;	/* 2.5 ms (per retry, max.) */
      do
	{
	  _delay_us (25);

	  if (dc3840_reply_ptr < 8)
	    continue;		/* Reply not yet complete. */

	  /* Check whether we received an ACK for the right command. */
	  if (dc3840_reply_buf[3] == DC3840_CMD_ACK
	      && dc3840_reply_buf[4] == a)
	    {
	      DC3840_DEBUG ("ACK!\n");
	      return 0;		/* Success! */
	    }

	  if (dc3840_reply_buf[3] == DC3840_CMD_NAK)
	    {
	      unsigned char err = dc3840_reply_buf[6]; (void) err;
	      DC3840_DEBUG ("NAK[%02x]!\n", err);
	    }
	  break;		/* Maybe resend. */
	}
      while (-- timeout);
    }
  while (-- retries);

  DC3840_DEBUG ("Timeout :(\n");
  return 1;			/* Fail. */
}


#ifdef DC3840_UDP_DEBUG

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"

static void
dc3840_net_main (void)
{
    if (!uip_newdata ())
	return;

    uip_udp_conn_t dc3840_conn;

    #define BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))
    uip_ipaddr_copy(dc3840_conn.ripaddr, BUF->srcipaddr);
    dc3840_conn.rport = BUF->srcport;
    dc3840_conn.lport = HTONS(DC3840_PORT);

    uip_udp_conn = &dc3840_conn;

    uint16_t offset = atoi (uip_appdata);
    dc3840_get_data (uip_appdata, offset, 512);
    uip_slen = 512;
    uip_process (UIP_UDP_SEND_CONN);
    router_output ();

    uip_slen = 0;		/* No reply. */
}

#endif	/* DC3840_UDP_DEBUG */


#include <avr/wdt.h>
void
dc3840_init (void)
{
  usart_init ();

  uint8_t sync_retries = 23;
  do
    {
      /* Send SYNC sequence.  dc3840_send_command internally repeats
	 three times, i.e. we send 23*5=115 sync requests  max. */
      if (dc3840_send_command (DC3840_CMD_SYNC, 0, 0, 0, 0))
	continue;

      /* Wait some more for next eight bytes to arrive. */
      wdt_kick ();
      _delay_ms (1);
      if (dc3840_reply_ptr >= 16) break;
    }
  while (-- sync_retries);

  if (! sync_retries)
    {
      DC3840_DEBUG ("Failed to sync to camera.\n");
      return;
    }

  /* We have received 16 bytes,
     -> ACK for our SYNC
     -> SYNC command from camera.

     We need to ACK the SYNC first. */
  dc3840_send_command (DC3840_CMD_ACK, DC3840_CMD_SYNC, 0, 0, 0);
  DC3840_DEBUG ("Successfully sync'ed to camera!\n");

#ifdef DC3840_UDP_DEBUG
  uip_ipaddr_t ip;
  uip_ipaddr_copy (&ip, all_ones_addr);

  uip_udp_conn_t *udp_dc3840_conn = uip_udp_new (&ip, 0, dc3840_net_main);

  if (!udp_dc3840_conn)
    return; /* dammit. */

  uip_udp_bind (udp_dc3840_conn, HTONS (DC3840_PORT));
#endif
}


#define dc3840_do(a,b...)						\
  if (dc3840_send_command (a,b))					\
    { DC3840_DEBUG ("dc3840 cmd failed: " # a "\n"); return 1; }

uint8_t
dc3840_capture (void)
{
  /* Reset configuration. */
  dc3840_do (DC3840_CMD_RESET, DC3840_RESET_STATES, 0, 0, 0);

  /* Configure camera */
  dc3840_do (DC3840_CMD_INITIAL, 1, DC3840_PREVIEW_JPEG, 9,
	     DC3840_RESOLUTION);

#ifdef DC3840_HIGH_COMPRESSION
  dc3840_do (DC3840_CMD_COMPRESS, DC3840_COMPRESS_HIGH, 0, 0, 0);
#else
  dc3840_do (DC3840_CMD_COMPRESS, DC3840_COMPRESS_LOW, 0, 0, 0);
#endif

#ifdef DC3840_BLACK_WHITE
  dc3840_do (DC3840_CMD_COLOR, DC3840_COLOR_BW, 0, 0, 0);
#else
  dc3840_do (DC3840_CMD_COLOR, DC3840_COLOR_RGB, 0, 0, 0);
#endif

  /* Acquire snapshot (stored to camera memory). */
  dc3840_data_length = 0;
  dc3840_do (DC3840_CMD_SNAPSHOT, 0, 0, 0, 0);

  return 0;			/* Success. */
}


#ifdef ECMD_PARSER_SUPPORT
int16_t
parse_cmd_dc3840_sync (char *cmd, char *output, uint16_t len)
{
  dc3840_init ();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_dc3840_capture (char *cmd, char *output, uint16_t len)
{
  dc3840_capture ();
  return ECMD_FINAL_OK;
}

int16_t
parse_cmd_dc3840_send (char *cmd, char *output, uint16_t len)
{
  char *ptr;

  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

#define tokenize(a)						\
  if (!(ptr = strchr (cmd, ' '))) return ECMD_ERR_PARSE_ERROR;	\
  *(ptr ++) = 0;						\
  uint8_t a = atoi (cmd);					\
  cmd = ptr;			/* forward command to start of next token */

  tokenize (a);
  tokenize (b);
  tokenize (c);
  tokenize (d);
  uint8_t e = atoi (cmd);

  return dc3840_send_command(a, b, c, d, e) ? ECMD_ERR_PARSE_ERROR : ECMD_FINAL_OK;
}

int16_t
parse_cmd_dc3840_light (char *cmd, char *output, uint16_t len)
{
  uint8_t level=0; // normal
  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

  if (cmd[0]=='1')
    level=1; // dunkel

  return dc3840_send_command(DC3840_CMD_LIGHT, level, 0, 0, 0) ? ECMD_ERR_PARSE_ERROR : ECMD_FINAL_OK;
}
int16_t
parse_cmd_dc3840_zoom (char *cmd, char *output, uint16_t len)
{
  uint8_t zoom=1; // default ohne zoom
  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

  if (cmd[0]=='1')
    zoom=0; // 2x zoom

  return dc3840_send_command(DC3840_CMD_DIG_ZOOM, zoom, 0, 0, 0) ? ECMD_ERR_PARSE_ERROR : ECMD_FINAL_OK;
}


#endif	/* ECMD_PARSER_SUPPORT */


ISR(usart(USART,_RX_vect))
{
  uint8_t temp = usart (UDR);

  if (dc3840_reply_ptr < 16)
    dc3840_reply_buf[dc3840_reply_ptr] = temp;

  else if (dc3840_capture_start)
    dc3840_capture_start --;

  else if (dc3840_capture_len)
    {
      *(dc3840_capture_ptr ++) = temp;
      dc3840_capture_len --;
    }

  dc3840_reply_ptr ++;
}


/* Store LEN bytes of image data to DATA, starting with OFFSET */
uint8_t
dc3840_get_data (uint8_t *data, uint16_t offset, uint16_t len)
{
  dc3840_capture_ptr = data;
  dc3840_capture_start = offset;
  dc3840_capture_len = len;

  /*  for (uint8_t i = 0; i < 200; i ++) {
    wdt_kick ();
    _delay_ms(10);
    } */

  if (dc3840_send_command (DC3840_CMD_GET_PICTURE,
			   DC3840_PICT_TYPE_SNAPSHOT, 0, 0, 0))
    return 1;			/* Failed to fetch image data. */

  _delay_ms (5);
  if (dc3840_reply_ptr < 16)
    {
      DC3840_DEBUG ("dc3840_reply_ptr is %d, only :(\n",
		    dc3840_reply_ptr);
      return 1;
    }

  /* In dc3840_reply_buf we now have:
     -> ACK for our GET_PICTURE		[00..07]
     -> DATA header			[08..0F] */
  if (dc3840_reply_buf[8 + 3] != DC3840_CMD_DATA)
    {
      DC3840_DEBUG ("expected DATA reply, found 0x%02x\n",
		    dc3840_reply_buf[8 + 3]);
      return 1;
    }

  if (dc3840_reply_buf[8 + 4] != DC3840_DATA_TYPE_JPEG)
    {
      DC3840_DEBUG ("expect JPEG data, found 0x%02x\n",
		    dc3840_reply_buf[8 + 4]);
      return 1;
    }

  dc3840_data_length = (dc3840_reply_buf[8 + 6] << 8)
    | dc3840_reply_buf[8 + 5];
  DC3840_DEBUG ("Image size: %u bytes\n", dc3840_data_length);

  /* Wait for data to be captured. */
  uint8_t timeout = 200;
  //while (dc3840_capture_len && --timeout) _delay_ms (5);
  while (dc3840_reply_ptr - 16 < dc3840_data_length
	 && --timeout)
    _delay_ms(5);

  if (!timeout)
    {
      DC3840_DEBUG ("Timeout capturing :(, ptr=%u\n", dc3840_reply_ptr);
      return 1;
    }

  return 0;
}


/*
  -- Ethersex META --
  header(hardware/camera/dc3840.h)
  init(dc3840_init)
  block([[Dc3840_camera|DC3840 mobil camera support]])
  ecmd_feature(dc3840_capture, "dc3840 capture",, Take a picture.  Access 'dc3840' via VFS afterwards.  See [[DC3840 Camera]] for details.)
  ecmd_feature(dc3840_send, "dc3840 send ", A B C D E, Send provided command bytes to the camera.)
  ecmd_feature(dc3840_sync, "dc3840 sync",, Re-sync to the camera)
  ecmd_feature(dc3840_light, "dc3840 light",, Light level of camera)
  ecmd_feature(dc3840_zoom, "dc3840 zoom",, Enable zoom of camera)
*/
