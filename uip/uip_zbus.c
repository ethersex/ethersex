/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#include "uip.h"
#include "uip_zbus.h"

#if defined(ZBUS_SUPPORT) && UIP_MULTI_STACK

STACK_DEFINITIONS(zbus_stack);

void 
zbus_stack_init (void)
{
  uip_ipaddr_t ip;

  CONF_ZBUS_IP;
  uip_sethostaddr(ip);

#ifdef IPV6_SUPPORT
  uip_setprefixlen(CONF_ZBUS_IP6_PREFIX_LEN);
#else
  CONF_ZBUS_IP4_NETMASK;
  uip_setnetmask(ip);
#endif
}

#endif /* ZBUS_SUPPORT && UIP_MULTI_STACK */
