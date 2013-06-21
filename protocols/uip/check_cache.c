/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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

#include "config.h"
#include "uip.h"
#include "uip_neighbor.h"
#include "uip_arp.h"
#include "check_cache.h"

uint8_t
uip_check_cache(uip_ipaddr_t *ripaddr) {
#ifdef ETHERNET_SUPPORT
    uip_ipaddr_t ipaddr;
#if ARCH == ARCH_HOST
    uip_stack_set_active(STACK_TAP);
#else
    uip_stack_set_active(STACK_ENC);
#endif

#ifdef IPV6_SUPPORT

    if (memcmp(ripaddr, uip_hostaddr, 8))
        /* Remote address is not on the local network, use router */
        uip_ipaddr_copy(&ipaddr, uip_draddr);
    else
        /* Remote address is on the local network, send directly. */
        uip_ipaddr_copy(&ipaddr, ripaddr);

    if (uip_ipaddr_cmp(&ipaddr, &all_zeroes_addr))
        return 1; /* Cowardly refusing to send IPv6 packet to :: */

    if (uip_neighbor_lookup(ipaddr))
        return 0;

#else  /* IPV4_SUPPORT */

    if (!uip_ipaddr_maskcmp(ripaddr, uip_hostaddr, uip_netmask))
        /* Remote address is not on the local network, use router */
        uip_ipaddr_copy(&ipaddr, uip_draddr);
    else
        /* Remote address is on the local network, send directly. */
        uip_ipaddr_copy(&ipaddr, ripaddr);

    /* uip_arp_lookup returns a pointer if the mac is in the arp cache */
    if (uip_arp_lookup(ipaddr))
        return 0;

#endif /* !IPV6_SUPPORT */
    return 1;
#endif /* ETHERNET_SUPPORT */

    return 0;
}

