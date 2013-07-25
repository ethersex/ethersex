/*
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef UIP_PARSE_H
#define UIP_PARSE_H

int16_t print_ipaddr(uip_ipaddr_t *addr, char *output, uint16_t len);
int8_t parse_ip(char *cmd, uip_ipaddr_t *ptr);

int16_t print_mac(struct uip_eth_addr *mac, char *output, uint16_t len);
int8_t parse_mac(char *cmd, struct uip_eth_addr *mac);

#endif /* UIP_PARSE_H */
