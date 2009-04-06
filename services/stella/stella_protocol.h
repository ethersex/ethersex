/*
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

#ifndef _STELLA_PROTOCOL_H
#define _STELLA_PROTOCOL_H

/* Represents a complete cron job */
struct stella_cron_event_struct
{
	union{
		int8_t fields[5];
		struct {
			int8_t minute;
			int8_t hour;
			int8_t day;
			int8_t month;
			int8_t dayofweek;
		};
	};
	uint8_t times;
	char appid;
	uint8_t extrasize;
};

/* Used to transfer complete cron jobs from ram to
* a client and to receive cron jobs from clients
* and to count cronjobs. */
struct stella_cron_struct
{
	uint8_t count;
};

void stella_protocol_parse(char* buf, uint8_t len);


#endif
