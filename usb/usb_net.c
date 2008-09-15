/* vim:fdm=marker et ai
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

#ifdef USB_NET_SUPPORT

static uint16_t recv_count;
static uint16_t recv_len;

uint8_t 
usb_net_setup(uint8_t  data[8]) 
{
  usbRequest_t *rq = (void *)data;
  if (rq->bRequest == USB_REQUEST_NET_SEND) {
    recv_len = rq->wValue.word;
    recv_count = 0;
    if (uip_buf_lock()) { /* Locking des buffers ist fehlgeschlagen */
      return 0;
    }
  }
  return USB_NO_MSG;
}

/* Host sends data to the device */
uint8_t
usb_net_write(uint8_t *data, uint8_t len)
{
  // FIXME
#if 0
  if ((recv_count + len) >) {
    len = ECMD_USB_BUFFER_LEN + 1 - recv_count;
    data[ECMD_USB_BUFFER_LEN] = 0;
  }
#endif

  memcpy(((uint8_t *) uip_buf ) + recv_count, data, len);
  recv_count += len;

  if (recv_count >= recv_len) {
    return 1;
  }
  return 0;
}

/* Host requests data from the device */
uint8_t
usb_net_read(uint8_t *data, uint8_t len)
{
  return 0;
}

void
usb_net_periodic(void)
{
  if (recv_len && (recv_count >= recv_len)) {
    /* A packet arrived, put it into uip */
    uip_len = recv_len;
    recv_len = 0;
    uip_input();
    uip_buf_unlock();
  }
}

#endif
