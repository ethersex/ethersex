/*
 *
 * Copyright (c) 2007,2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2011-2012 by Erik Kunze <ethersex@erik-kunze.de>
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef __CLOCK_H
#define __CLOCK_H

#include <inttypes.h>
#include "config.h"
#include "services/clock/clock_lib.h"

void clock_init(void);
void clock_periodic(void);
void clock_tick(void);

/* the actual time as unix time stamp */
timestamp_t clock_get_time(void);

/* when was the clock synced the last time (unix timestamp) */
timestamp_t clock_last_sync(void);

/* when was the clock synced the last time (ticks) */
timestamp_t clock_last_sync_tick(void);

/* last delta time (from unix timestamp) */
int16_t clock_last_delta(void);

/* DCF syncs in Folge */
uint16_t clock_dcf_count(void);
void set_dcf_count(const uint16_t new_dcf_count);

/* NTP syncs in Folge */
uint16_t clock_ntp_count(void);
void set_ntp_count(const uint16_t new_ntp_count);

/* the actual ntp_timer */
uint16_t clock_last_ntp(void);

/* how long is the system up (seconds) */
timestamp_t clock_get_uptime(void);

/* the actual time */
void clock_set_time_raw(timestamp_t new_sync_timestamp);
void clock_set_time_raw_hr(timestamp_t new_sync_timestamp, uint8_t new_ticks);
void clock_set_time(timestamp_t new_sync_timestamp);

/* get tick counter */
uint8_t clock_get_ticks(void);

#endif /* __CLOCK_H */
