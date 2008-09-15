/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (C) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "usbdrv/usbdrv.h"
#include "../ecmd_parser/ecmd.h"
#include "requests.h"
#include "../config.h"
#include "usb_net.h"

#ifdef USB_NET_SUPPORT

static uint16_t usb_rq_index;
static uint16_t usb_rq_len;

uint8_t usb_packet_ready;

uint8_t 
usb_net_setup(uint8_t  data[8]) 
{
  usbRequest_t *rq = (void *)data;

  if (rq->bRequest == USB_REQUEST_NET_SEND) {
    if (uip_buf_lock())	  /* Unable to aquire lock, ignore packet. */
      return 0;

    usb_rq_index = 0;
    usb_rq_len = rq->wValue.word;
  }
  else if (usb_packet_ready) {
    usbMsgPtr = uip_buf;
    return uip_len;
  }
  else
    return 0;

  return USB_NO_MSG;
}

void
usb_net_read_finished (void)
{
  uip_buf_unlock ();
  usb_packet_ready = 0;
}

/* Host sends data to the device */
uint8_t
usb_net_write(uint8_t *data, uint8_t len)
{
  // FIXME
#if 0
  if ((usb_rq_index + len) >) {
    len = ECMD_USB_BUFFER_LEN + 1 - usb_rq_index;
    data[ECMD_USB_BUFFER_LEN] = 0;
  }
#endif

  memcpy(((uint8_t *) uip_buf ) + usb_rq_index, data, len);
  usb_rq_index += len;

  if (usb_rq_index >= usb_rq_len) {
    return 1;
  }
  return 0;
}

void
usb_net_txstart (void)
{
  usb_packet_ready = 1;
}

void
usb_net_periodic(void)
{
  if (usb_rq_len && (usb_rq_index >= usb_rq_len)) {
    /* A packet arrived, put it into uip */
    uip_len = usb_rq_len;
    usb_rq_len = 0;
    uip_input();

    if (uip_len == 0)
      uip_buf_unlock ();	/* The stack didn't generate any data
				   that has to be sent back. */

    else
      fill_llh_and_transmit ();	/* Application has generated output,
				   send it out. */
  }
}

#endif
