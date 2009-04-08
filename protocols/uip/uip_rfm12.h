/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef UIP_RFM12_H
#define UIP_RFM12_H

#include "config.h"
#if defined(RFM12_IP_SUPPORT) && defined(ROUTER_SUPPORT)

#include "uip-conf.h"

void rfm12_stack_init (void);


#endif /* RFM12_IP_SUPPORT and ROUTER_SUPPORT */
#endif /* not UIP_RFM12_H */
