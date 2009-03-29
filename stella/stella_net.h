/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
 }}} */

#include "../uip/uip.h"

#ifndef _STELLA_NET_H
#define _STELLA_NET_H

/* This header structure is infront of every response
 * coming via the stella protocol */
struct stella_response_header
{
	char protocol;
	char cmd;
};

/* Used to inform clients about the channel state.
 * The protocol version is also propagated here */
struct stella_response_detailed_struct
{
	struct stella_response_header id;
	uint8_t protocol_version;
	uint8_t channel_count;
	uint8_t pwm_channels[8];
};

/* Represents a complete cron job */
struct stella_cron_event_struct
{
	int8_t fields[5];
	uint8_t times;
	char appid;
	uint8_t extrasize;
};

/* Used to transfer complete cron jobs from ram to
 * a client and to receive cron jobs from clients
 * and to count cronjobs. */
struct stella_cron_struct
{
	struct stella_response_header id;
	uint8_t count;
};

void stella_net_init(void);
void stella_net_main(void);
void stella_net_unicast(uint8_t len);
void stella_net_broadcast(uint8_t len);
#endif
