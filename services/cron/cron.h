/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _CRON_H
#define _CRON_H

/* meaning of the signed values in cron_event_t:
 *   x in 0..59:    absolute value (minute)
 *   x in 0..23:    absolute value (hour)
 *   x in 0..30:    absolute value (day)
 *   x in 0..12:    absolute value (month)
 *   x in 0..6 :    absolute value (dow) // day of the week
 *   x is    -1:    wildcard
 *   x in -59..-2:  Example -2 for hour: when hour % 2 == 0 <=> every 2 hours
 */

struct cron_event_t {
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
	// how many times should this job be executed? 0=unlimited
	uint8_t times;
	// One-Byte identifier of the application, eg 'S' for a Stella job.
	// Use 0 if you don't care. This is exspecially important for the
	// gui control application to be able to list jobs of just one appid.
	char appid;

	void (*handler)(void* data);
	// next,prev pointer for double linked lists;
	// last entry's next is NULL, heads prev is NULL
	struct cron_event_t* next;
	struct cron_event_t* prev;
	/* Cron jobs can save a pointer to extra user data for applications.
	 * We don't save the data length though. Applications are in charge
	 * to manage that. The memory had to be allocated with malloc on the heap,
	 * because we will free the memory of the extra data when this cronjob gets
	 * removed. */
	void* extradata;
};

extern struct cron_event_t* head;
extern struct cron_event_t* tail;
extern uint8_t cron_use_utc;

#define USE_UTC 1
#define USE_LOCAL 0
#define INFINIT_RUNNING 0

/** add cron job to the linked list */
void cron_jobadd(
	void (*handler)(void* data),	// cackback to application
	char appid,							// application id (use NULL if you don't care)
	int8_t minute, int8_t hour,	// time
	int8_t day, int8_t month, int8_t dayofweek, //date
	uint8_t times,						// repeat>0 or INFINIT_RUNNING
	void* extradata					// malloc allocated extra data (maybe NULL)
);

/** remove the job from the linked list */
void cron_jobrm(struct cron_event_t* job);

/** count jobs */
uint8_t cron_jobs();

/** get a pointer to the entry of the cron job's linked list at position jobposition */
struct cron_event_t* cron_getjob(uint8_t jobposition);

/** init cron. (Set head to NULL for example) */
void cron_init(void);

/** periodically check, if an event matches the current time. must be called
  * once per minute */
void cron_periodic(void);

#endif
