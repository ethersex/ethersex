/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2019 by Erik Kunze <ethersex@erik-kunze.de>
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

#include "config.h"
#include "core/param.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/ecmd/parser.h"
#include "protocols/usb/ecmd_usb.h"
#include "protocols/usb/usbdrv/usbdrv.h"


#define ECMD_USB_BUFFER_LEN 50

static char recv_buffer[ECMD_USB_BUFFER_LEN + 1];
static uint8_t recv_index;

static char send_buffer[ECMD_USB_BUFFER_LEN + 1];
static uint8_t send_remain;
static uint8_t send_index;
static uint8_t parse_again;


static void
emcd_usb_parse(void)
{
  int16_t len = ecmd_parse_command(recv_buffer, send_buffer,
                                   ECMD_USB_BUFFER_LEN);

  /* Check if the parse has to be called again */
  if ((parse_again = is_ECMD_AGAIN(len)) != 0)
  {
    len = ECMD_AGAIN(len);
    if (len > 0)
    {
      if (send_buffer[len] != ECMD_NO_NEWLINE)
        send_buffer[len++] = '\n';
    }
  }
  else /* Clear receive buffer */
  {
    memset(recv_buffer, 0, ECMD_USB_BUFFER_LEN);
    recv_index = 0;
  }
  send_remain = len;
  send_index = 0;
}

usbMsgLen_t
ecmd_usb_setup(usbRequest_t * rq)
{
  if (rq->wValue.word == 1)     /* Write */
    recv_index = 0;
  else                          /* Read */
    emcd_usb_parse();
  return USB_NO_MSG;
}

/* Host sends data to the device */
uchar
ecmd_usb_write(uint8_t * data, uint8_t len)
{
  if ((recv_index + len) > ECMD_USB_BUFFER_LEN)
  {
    len = ECMD_USB_BUFFER_LEN + 1 - recv_index;
    data[ECMD_USB_BUFFER_LEN] = 0;
  }

  memcpy(recv_buffer + recv_index, data, len);
  recv_index += len;

  return (recv_buffer[recv_index - 1] == 0);
}

/* Host requests data from the device */
uchar
ecmd_usb_read(uint8_t * data, uint8_t len)
{
  uint8_t send = MIN(send_remain, len);
  if (send > 0)
  {
    memcpy(data, send_buffer + send_index, send);
    send_remain -= send;
    send_index += send;
  }
  if (send < len && parse_again)
  {
    emcd_usb_parse();
    send += ecmd_usb_read(data + send, len - send);
  }

  return send;
}
