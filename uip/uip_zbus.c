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

/* We're now compiling the outer side of the uIP stack */
#define ZBUS_OUTER
#define STACK_NAME(a) zbus_stack_ ## a

#include "uip_zbus.h"

#if defined(ZBUS_SUPPORT) && defined(ENC28J60_SUPPORT)

/* We're set to compile multi stack now ... */
#include "uip.c"

void 
zbus_stack_init (void)
{
  uip_ipaddr_t ip;

  CONF_ZBUS_IP;
  uip_sethostaddr(ip);
}

#endif /* ZBUS_SUPPORT && ENC28J60_SUPPORT */
