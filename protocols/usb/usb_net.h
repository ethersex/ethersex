/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef _USB_NET_H
#define _USB_NET_H

#include "protocols/usb/usbdrv/usbdrv.h"

/* Whether there is a packet stored in uip_buf that has to be sent
   to the host. */
extern uint8_t usb_packet_ready;

#ifdef USB_NET_SUPPORT
#  define usb_net_tx_active() (usb_packet_ready != 0)
#else
#  define usb_net_tx_active() (0)
#endif

usbMsgLen_t usb_net_setup(uint8_t  data[8]);
uint8_t usb_net_write(uint8_t *data, uint8_t len);
uint8_t usb_net_read(uint8_t *data, uint8_t len);
void usb_net_read_finished(void);

/* Initialize USB network stack. */
void usb_net_init (void);

/* Request the packet, stored in uip_buf, to be sent to USB host.
   uip_buf must be locked and will be unlocked automatically. */
void usb_net_txstart(void);

void usb_net_periodic(void);

#endif /* _USB_NET_H */
