/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2011 by Martin Wache <M.Wache@gmx.net>
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
 */

#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "usbdrv/usbdrv.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "requests.h"
#include "config.h"

#ifdef ECMD_USB_SUPPORT

/*
 * The new protocol used by ecmd_usb uses tokens (cmd_count and reply_count)
 * to confirm the reception of buffers, this avoids loosing commands or
 * replies. The hosts uses the two bytes index field of the setup packet to
 * send the cmd token, and to confirm reply tokens, while the device
 * sends the cmd token confirmation in the first byte of the reply,
 * and the reply token in the second byte of the reply.
 * Notice that for some reason (endianess?) the order of the index
 * bytes for host and device is reversed.
 * As long as host or device don't get a confirmation for the last buffer,
 * they will repeat sending the last buffer.
 */

#define ECMD_USB_BUFFER_LEN 50

static uint8_t cmd_count=255;
static uint8_t reply_count=255;

static uint8_t cmd_len;
static uint8_t recv_count;
static char recv_buffer[ECMD_USB_BUFFER_LEN + 1];

static char send_buffer[ECMD_USB_BUFFER_LEN];
static uint8_t send_buf_len;

uint8_t
ecmd_usb_setup(uint8_t  data[8])
{
  usbRequest_t *rq = (void *)data;
  if (rq->wIndex.bytes[1] == reply_count )
    send_buf_len = 0;
  if (rq->wValue.word == 1) { /* Write */
    if ( cmd_len || send_buf_len > 0 )
      /* last command not yet consumed or reply buffer not empty */
      return 0;
    recv_count = 0;
    if (rq->wLength.word > ECMD_USB_BUFFER_LEN )
      cmd_len = ECMD_USB_BUFFER_LEN;
    else
      cmd_len = rq->wLength.word;
    cmd_count = rq->wIndex.bytes[0];
  } else { /* Read */
    if ( send_buf_len > rq->wLength.word )
      /* host requests only a part of the response - will loose the rest */
      send_buf_len = rq->wLength.word;
    usbMsgPtr = &send_buffer;
    return send_buf_len;
  }
  return USB_NO_MSG;
}

void
usb_ecmd_periodic()
{
  if (cmd_len && cmd_len<=recv_count && send_buf_len == 0)
  {
    /* command to parse, end return buffer is empty */
    int16_t ret = ecmd_parse_command(recv_buffer,
                                     send_buffer+2,
                                     sizeof(send_buffer)-2);
    reply_count++;
    send_buffer[0]=cmd_count;
    send_buffer[1]=reply_count;
    if ( is_ECMD_AGAIN(ret) )
    {
      send_buf_len = ECMD_AGAIN(ret);
    } else {
      /* command is consumed */
      send_buf_len = ret;
      cmd_len = 0;
    }
    send_buf_len +=2;
    // FIXME ECMD_Err?
  }
}

/* Host sends data to the device */
uint8_t
ecmd_usb_write(uint8_t *data, uint8_t len)
{
  if ((recv_count + len) > ECMD_USB_BUFFER_LEN) {
    len = ECMD_USB_BUFFER_LEN - recv_count;
  }

  memcpy(recv_buffer + recv_count, data, len);
  recv_count += len;

  if ( cmd_len <= recv_count )
  {
    recv_buffer[recv_count] = 0;
    return 1;
  }
  return 0;

}

#endif
