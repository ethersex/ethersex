/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#ifndef _STELLA_NET_H
#define _STELLA_NET_H

/* This header structure is infront of every response
 * coming via the stella protocol */
struct stella_response_header
{
	char protocol;
	char cmd;
};
#define STELLA_HEADER 2

/* Used to inform clients about the channel state.
 * The protocol version is also propagated here */
struct stella_response_detailed_struct
{
	uint8_t protocol_version;
	uint8_t channel_count;
	uint8_t pwm_channels[8];
};

void stella_net_init(void);
void stella_net_main(void);
void stella_net_unicast(uint8_t len);
void stella_net_broadcast(uint8_t len);
void stella_net_ack(void);
void stella_net_wb_getvalues(const uint8_t msgtype);
void* stella_net_response(const uint8_t msgtype);
#endif
