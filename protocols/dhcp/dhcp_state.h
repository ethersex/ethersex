/*
 *
 * Copyright (c) 2009 by Dirk Tostmann <tostmann@busware.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef DHCP_STATE_H
#define DHCP_STATE_H

/* state */
struct dhcp_connection_state_t {
    char state;
  
    uint8_t serverid[4];
    
    uint16_t lease_time[2];
    uint16_t ipaddr[2];
    uint16_t netmask[2];
    uint16_t dnsaddr[2];
    uint16_t default_router[2];
    uint16_t ntpaddr[2];

    uint8_t retry_timer;
    uint8_t retry_counter;
};

#endif /* DHCP_STATE_H */
