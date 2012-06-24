/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 * Copyright (c) 2010 by iT Engineering Stefan Müller <mueller@ite-web.de>
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

#include "services/cron/cron_shared.h"

#include "config.h"

#if defined(CRON_VFS_SUPPORT) || defined(CRON_EEPROM_SUPPORT)
#define CRON_PERSIST_SUPPORT 1
#endif

/** This structure represents a cron job */
struct cron_event
{
  uint8_t extrasize;
  cron_conditions_t cond;
  uint8_t repeat;
        /** Either CRON_JUMP or CRON_ECMD */
  unsigned cmd:4;
  unsigned use_utc:1;
#ifdef CRON_PERSIST_SUPPORT
  unsigned persistent:1;
#endif
#ifdef CRON_ANACRON_SUPPORT
  unsigned anacron:1;
  unsigned anacron_pending:1;
#endif
  union
  {
                /** for CRON_JUMP
		* All additional bytes are extra user data for applications. E.g.
		* additional arguments for the CRON_JUMP.
		* The memory for "extradata" has to be allocated with malloc on the heap,
		* because we will free the memory "extradata" on cronjob removal. */
    struct
    {
      void (*handler) (void *);
      char extradata;
    };
    // for CRON_ECMD
    struct
    {
      char ecmddata;
    };
  };
};
#define cron_event_size (sizeof(struct cron_event))

/** This structure is used for the double linked list of cronjobs */
struct cron_event_linkedlist
{
  // next,prev pointer for double linked lists;
  // last entry's next is NULL, heads prev is NULL
  struct cron_event_linkedlist *next;
  struct cron_event_linkedlist *prev;
  struct cron_event event;
};

extern struct cron_event_linkedlist *head;
extern struct cron_event_linkedlist *tail;

#define INFINIT_RUNNING 0
#define CRON_APPEND -1

#define CRON_JUMP 1
#define CRON_ECMD 2


/**
 * @brief Insert cron job (that invokes a callback function) to the linked list.
 * @param minute minute of trigger time
 * @param hour hour of trigger time
 * @param day day of trigger date
 * @param month month of trigger date
 * @param daysofweek bitmask of selected weekdays of trigger date
 * @param repeat repeat>0 or INFINIT_RUNNING
 * @param position -1 to append else the new job is inserted at that position
 * @param handler callback function with signature "void func(void* data)"
 * @param extrasize size of extra data that is passed to the callback function
 * @param extradata pointer to extra data that is passed to the callback function
 * @return position where job is inserted, -1 in case of error
 */
int16_t cron_jobinsert_callback(int8_t minute, int8_t hour, int8_t day,
                                int8_t month, int8_t daysofweek,
                                uint8_t repeat, int8_t position,
                                void (*handler) (void *), uint8_t extrasize,
                                void *extradata);

/**
 * @brief Insert cron job (that will get parsed by the ecmd parser) to the linked list.
 * @param minute minute of trigger time
 * @param hour hour of trigger time
 * @param day day of trigger date
 * @param month month of trigger date
 * @param daysofweek bitmask of selected weekdays of trigger date
 * @param repeat repeat>0 or INFINIT_RUNNING
 * @param position -1 to append else the new job is inserted at that position
 * @param cmddata ecmd string (cron will not free memory but just copy from pointerposition! Has to be null terminated.)
 * @return position where job is inserted, -1 in case of error
 */
int16_t cron_jobinsert_ecmd(int8_t minute, int8_t hour, int8_t day,
                            int8_t month, int8_t daysofweek, uint8_t repeat,
                            int8_t position, char *ecmd);

#ifdef CRON_PERSIST_SUPPORT
/** Saves all as persistent marked Jobs to vfs */
int16_t cron_save();
#endif

/**
 * @brief Insert cron job to the linked list.
 * @param newone The new cron job structure (malloc'ed memory!)
 * @param position Where to insert the new job
 * @return position where job is inserted
 */
uint8_t cron_insert(struct cron_event_linkedlist *newone, int8_t position);

/** remove the job from the linked list */
void cron_jobrm(struct cron_event_linkedlist *job);

/** count jobs */
uint8_t cron_jobs();

/** get a pointer to the entry of the cron job's linked list at position jobposition */
struct cron_event_linkedlist *cron_getjob(uint8_t jobposition);

/** init cron. (Set head to NULL for example) */
void cron_init(void);

/**
 * @brief execute cron job
 * @param exec Item to execute
 */
void cron_execute(struct cron_event_linkedlist *exec);

/** periodically check, if an event matches the current time. must be called
  * once per minute */
void cron_periodic(void);

#endif /* _CRON_H */
