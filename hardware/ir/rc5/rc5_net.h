/*
 *         simple rc5 udp implementation
 *
 *    for additional information please
 *    see http://lochraster.org/etherrape
 *
 * (c) by Mario Goegel <mario@goegel.net>
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


#ifndef RC5_NET_H
#define RC5_NET_H

#include <stdint.h>

#ifdef RC5_UDP_SUPPORT

/* UDP constants */
#define RC5_UDPPORT 6669

/* prototypes */
void rc5_net_init(void);
void rc5_udp_send(void);
void rc5_udp_recv(void);
uint8_t rc5_check_cache(void);

#endif /* RC5_UDP_SUPPORT */
#endif /* RC5_NET_H */
