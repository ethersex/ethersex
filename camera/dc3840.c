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

#include "../config.h"

#ifdef DC3840_SUPPORT

#include <util/delay.h>
#include <string.h>
#include "dc3840.h"

/* USART cruft. */
#define USE_USART DC3840_USE_USART
#define BAUD 921600
#include "../usart.h"

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
static void dc3840_send_uart (uint8_t byte) __attribute__ ((noinline));

/* Send a command to the camera and wait for ACK (return 0).
   This function automatically repeats the command up to three times.
   Returns 1 on timeout or NAK. */
static uint8_t dc3840_send_command (uint8_t, uint8_t, uint8_t, uint8_t,
				    uint8_t)	__attribute__ ((noinline));


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
  syslog_sendf ("-> %02x %02x %02x %02x %02x\n", a, b, c, d, e);

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
	      unsigned char err = dc3840_reply_buf[6];
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
  dc3840_do (DC3840_CMD_INITIAL, 1, DC3840_PREVIEW_JPEG, 9, 5);

  /* Acquire snapshot (stored to camera memory). */
  dc3840_do (DC3840_CMD_SNAPSHOT, 0, 0, 0, 0);

  return 0;			/* Success. */
}


#ifdef ECMD_PARSER_SUPPORT
int16_t
parse_cmd_dc3840_sync (char *cmd, char *output, uint16_t len)
{
  dc3840_init ();
  return 0;
}

int16_t
parse_cmd_dc3840_capture (char *cmd, char *output, uint16_t len)
{
  dc3840_capture ();
  return 0;
}

int16_t
parse_cmd_dc3840_send (char *cmd, char *output, uint16_t len)
{
  char *ptr;

  /* ignore leading spaces */
  while (*cmd == ' ')
    cmd ++;

#define tokenize(a)				\
  if (!(ptr = strchr (cmd, ' '))) return -1;	\
  *(ptr ++) = 0;				\
  uint8_t a = atoi (cmd);			\
  cmd = ptr;			/* forward command to start of next token */

  tokenize (a);
  tokenize (b);
  tokenize (c);
  tokenize (d);
  uint8_t e = atoi (cmd);

  return dc3840_send_command (a, b, c, d, e) ? -1 : 0;
}
#endif	/* ECMD_PARSER_SUPPORT */


SIGNAL(usart(USART,_RX_vect))
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

#endif	/* DC3840_SUPPORT */
