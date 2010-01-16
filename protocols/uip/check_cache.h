/*
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

#include "uip.h"

#ifndef _UIP_CHECK_CACHE_H
#define _UIP_CHECK_CACHE_H

/**
 * uip_check_cache, checks if the arp/neighbour cache is valid for given ip
 * address 
 *
 * Example:
 \code
 uip_check_cache(udp_conn->ripaddr);
 \endcode
 *
 * \hideinitializer
 */

uint8_t uip_check_cache(uip_ipaddr_t *ripaddr);

#endif /* _UIP_CHECK_CACHE_H */
