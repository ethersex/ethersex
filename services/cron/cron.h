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

#include <stdint.h>

/** This structure represents a cron job */
struct cron_event {
	union{
		int8_t fields[5];
		/** meaning of the signed values in the following structure:
		  *   x in 0..59:    absolute value (minute)
		  *   x in 0..23:    absolute value (hour)
		  *   x in 0..30:    absolute value (day)
		  *   x in 0..12:    absolute value (month)
		  *   x in 0..6 :    absolute value (dow) // day of the week
		  *   x is    -1:    wildcard
		  *   x in -59..-2:  Example -2 for hour: when hour % 2 == 0 <=> every 2 hours */
		struct {
			int8_t minute;
			int8_t hour;
			int8_t day;
			int8_t month;
			int8_t dayofweek;
		};
	};
	uint8_t repeat;
	/** Either CRON_JUMP or CRON_ECMD */
	uint8_t cmd;
	union {
		/** for CRON_JUMP
		* All additional bytes are extra user data for applications. E.g.
		* additional arguments for the CRON_JUMP.
		* The memory for "extradata" has to be allocated with malloc on the heap,
		* because we will free the memory "extradata" on cronjob removal. */
		struct {
			void (*handler)(void*);
			void* extradata;
		};
		// for CRON_ECMD
		struct {
			void* ecmddata;
		};
	};
};
#define cron_event_size (sizeof(struct cron_event))

/** This structure is used for the double linked list of cronjobs */
struct cron_event_linkedlist
{
	struct cron_event event;
	// next,prev pointer for double linked lists;
	// last entry's next is NULL, heads prev is NULL
	struct cron_event_linkedlist* next;
	struct cron_event_linkedlist* prev;
};

extern struct cron_event_linkedlist* head;
extern struct cron_event_linkedlist* tail;
extern uint8_t cron_use_utc;

#define USE_UTC 1
#define USE_LOCAL 0
#define INFINIT_RUNNING 0
#define CRON_APPEND -1

#define CRON_JUMP 0
#define CRON_ECMD 0

/** Insert cron job (that invokes a callback function) to the linked list.
  * @minute, @hour: trigger time
  * @day, @month, @dayofweek: trigger date
  * @repeat: repeat>0 or INFINIT_RUNNING
  * @position: -1 to append else the new job is inserted at that position
  * @handler: callback function with signature "void func(void* data)"
  * @extrasize, @extradata: extra data that is passed to the callback function
  */
void cron_jobinsert_callback(
	int8_t minute, int8_t hour, int8_t day, int8_t month, int8_t dayofweek,
	uint8_t repeat, int8_t position, void (*handler)(void*), uint8_t extrasize, void* extradata
);

/** Insert cron job (that will get parsed by the ecmd parser) to the linked list.
* @minute, @hour: trigger time
* @day, @month, @dayofweek: trigger date
* @repeat: repeat>0 or INFINIT_RUNNING
* @position: -1 to append else the new job is inserted at that position
* @cmddata: ecmd string (cron will not free memory but just copy from pointerposition! Has to be null terminated.)
*/
void cron_jobinsert_ecmd(
	int8_t minute, int8_t hour, int8_t day, int8_t month, int8_t dayofweek,
	uint8_t repeat, int8_t position, char* ecmd
);

/** Insert cron job to the linked list.
* @newone: The new cron job structure (malloc'ed memory!)
* @position: Where to insert the new job
*/
void cron_insert(struct cron_event_linkedlist* newone,int8_t position);

/** remove the job from the linked list */
void cron_jobrm(struct cron_event_linkedlist* job);

/** count jobs */
uint8_t cron_jobs();

/** get a pointer to the entry of the cron job's linked list at position jobposition */
struct cron_event_linkedlist* cron_getjob(uint8_t jobposition);

/** init cron. (Set head to NULL for example) */
void cron_init(void);

/** periodically check, if an event matches the current time. must be called
  * once per minute */
void cron_periodic(void);

#endif /* _CRON_H */
