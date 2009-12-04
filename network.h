/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(C) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (version 3)
 * as published by the Free Software Foundation.
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

#ifndef _NETWORK_H
#define _NETWORK_H

#include "hardware/ethernet/enc28j60.h"
#include "config.h"

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_arp.h"
#include "protocols/uip/ipv6.h"

/* prototypes */

/* do all network initialization stuff */
void network_init(void);

/* check for ethernet controller interrupts */
void network_process(void);

#ifdef ENC28J60_SUPPORT
/* send a packet placed in the global buffer */
void transmit_packet(void);

static inline uint8_t enc28j60_txstart(void)
{
  uint8_t retval;

#if UIP_CONF_IPV6
  retval = uip_neighbor_out();
  if (uip_len)
#else
  retval = uip_arp_out();
#endif
  transmit_packet();

  return retval;
}
#endif

#endif /* _NETWORK_H */
