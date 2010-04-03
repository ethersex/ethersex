/*
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
 */

#include <string.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "usbdrv/usbdrv.h"
#include "requests.h"
#include "config.h"
#include "usb_net.h"

#ifdef USB_NET_SUPPORT

#ifdef UIP_MULTI_STACK
STACK_DEFINITIONS(usb_stack);
#endif

static uint16_t usb_rq_index;
static uint16_t usb_rq_len;

uint8_t usb_packet_ready;

usbMsgLen_t
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
    usbMsgPtr = uip_buf + USB_BRIDGE_OFFSET;
    return usb_rq_len;
  }
  else
    return 0;

  return USB_NO_MSG;
}

void
usb_net_read_finished (void)
{
  usb_packet_ready = 0;
  uip_buf_unlock ();
}

/* Host sends data to the device */
uint8_t
usb_net_write(uint8_t *data, uint8_t len)
{
  if (usb_rq_index + USB_BRIDGE_OFFSET + len < UIP_CONF_BUFFER_SIZE)
    memcpy(uip_buf + USB_BRIDGE_OFFSET + usb_rq_index, data, len);
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

  usb_rq_index = 0;
  usb_rq_len = uip_len;
}

void
usb_net_periodic(void)
{
  if (usb_rq_len && (usb_rq_index >= usb_rq_len)) {
    /* A packet arrived, put it into uip */
    uip_len = usb_rq_len + UIP_LLH_LEN;
    usb_rq_len = 0;
    router_input (STACK_USB);

    if (uip_len == 0)
      uip_buf_unlock ();	/* The stack didn't generate any data
				   that has to be sent back. */

    else
      router_output ();         /* Application has generated output,
				   send it out. */
  }
}

void
usb_net_init (void)
{
#ifdef UIP_MULTI_STACK
  uip_ipaddr_t ip;

  uip_stack_set_active (STACK_USB);

  set_CONF_USB_NET_IP(&ip);
  uip_sethostaddr (&ip);

#ifdef IPV6_SUPPORT
  uip_setprefixlen (CONF_USB_NET_IP6_PREFIX_LEN);
#else
  set_CONF_USB_NET_IP4_NETMASK(&ip);
  uip_setnetmask (&ip);
#endif
#endif /* UIP_MULTI_STACK */
}

#endif
