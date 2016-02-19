/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "cron.h"
#include "test.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/ecmd/parser.h"
#include "services/clock/clock.h"

#ifdef CRON_VFS_SUPPORT
#include "core/vfs/vfs.h"
#include "core/vfs/vfs-util.h"
#define CRON_FILENAME "crn.t"
#endif

uint32_t last_check;
struct cron_event_linkedlist *head;
struct cron_event_linkedlist *tail;

#ifdef CRON_PERSIST_SUPPORT
void
cron_load()
{
  uint8_t extrasize;
  uint16_t wsize;
  uint8_t count, i;
  struct cron_event_linkedlist *newone;

#ifdef CRON_VFS_SUPPORT
  struct vfs_file_handle_t *file;
  vfs_size_t position = sizeof(count);

  file = vfs_open(CRON_FILENAME);
  if (file == NULL)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: no cron file found!\n");
#endif
    return;
  }

  if (vfs_read(file, &count, sizeof(count)) != sizeof(count))
    goto end;
#else
  uint16_t position = sizeof(count);
  eeprom_restore_offset(crontab, 0, &count, sizeof(uint8_t));

  /* do not read a bogus crontab from empty eeprom */
  if (count == 0xFF)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: found bogus cronjob count 0xFF, reset count to zero\n");
#endif
    count = 0;
  }
#endif
#ifdef DEBUG_CRON
  debug_printf("cron: file with %i entries found\n", count);
#endif

  // read each cron event
  for (i = 0; i < count; i++)
  {
    // read extrasize
#ifdef CRON_VFS_SUPPORT
    if (vfs_read(file, &extrasize, sizeof(extrasize)) != sizeof(extrasize))
      goto end;
#else
    eeprom_restore_offset(crontab, position, &extrasize, sizeof(extrasize));
#endif
    // try to get ram space
    wsize = sizeof(struct cron_event_linkedlist) + extrasize;
    newone = malloc(wsize);

#ifdef DEBUG_CRON
    debug_printf
      ("cron: try to allocate size of %i consist of struct %i and extrasize %i!\n",
       wsize, sizeof(struct cron_event_linkedlist), extrasize);
#endif

    // no more ram available -> abort
    if (!newone)
    {
#ifdef DEBUG_CRON
      debug_printf("cron: not enough ram!\n");
#endif
      goto end;
    }

    wsize = sizeof(struct cron_event) + extrasize;
#ifdef CRON_VFS_SUPPORT
    if (vfs_fseek(file, position, SEEK_SET) != 0)
    {
      free(newone);
      newone = NULL;
      goto end;
    }
    if (vfs_read(file, &newone->event, wsize) != wsize)
    {
      free(newone);
      newone = NULL;
      goto end;
    }
#else
    eeprom_restore_offset(crontab, position, &newone->event, wsize);
#endif
    position += wsize;
    cron_insert(newone, CRON_APPEND);
  }
end:
#ifdef CRON_VFS_SUPPORT
  vfs_close(file);
#endif
  return;
}

#ifdef CRON_VFS_SUPPORT
int16_t
cron_write_error(struct vfs_file_handle_t * file)
{
  vfs_close(file);

  /* truncate file */
  vfs_close(vfs_open(CRON_FILENAME));
  return -1;
}
#endif

int16_t
cron_save()
{
#ifdef CRON_VFS_SUPPORT
  struct vfs_file_handle_t *file;
  vfs_size_t filesize;
  vfs_size_t tempsize;
#else
  uint16_t filesize;
  uint16_t tempsize;
#endif
  uint8_t count = 0;
  uint8_t saved_count = 0;

#ifdef DEBUG_CRON
  debug_printf("cron: saving jobs\n");
#endif

#ifdef CRON_VFS_SUPPORT
  file = vfs_create(CRON_FILENAME);

  if (file == NULL)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: can't create file\n");
#endif
    return -1;
  }
#endif

  // placeholder
  filesize = sizeof(count);
  struct cron_event_linkedlist *job = head;
  while (job)
  {
    if (job->event.persistent)
    {
      count++;
      filesize += sizeof(struct cron_event) + job->event.extrasize;
    }
    job = job->next;
  }

#ifdef CRON_VFS_SUPPORT
  if (vfs_write(file, &count, sizeof(count)) != sizeof(count))
    return cron_write_error(file);
#else
  if (filesize >= CRON_EEPROM_SIZE)
    return -1;
  eeprom_save_offset(crontab, 0, &count, sizeof(uint8_t));
#endif

  filesize = sizeof(count);
  job = head;
  while (job)
  {
    if (job->event.persistent)
    {
#ifdef DEBUG_CRON
      debug_printf("cron: writing job %i\n", count);
#endif

      tempsize = sizeof(struct cron_event) + job->event.extrasize;
#ifdef DEBUG_CRON
      debug_printf
        ("cron: try to allocate size of %i consist of struct %i and extrasize %i!\n",
         tempsize, sizeof(struct cron_event), job->event.extrasize);
#endif

#ifdef CRON_VFS_SUPPORT
      if (vfs_write(file, &job->event, tempsize) != tempsize)
        return cron_write_error(file);
#else
      eeprom_save_offset(crontab, filesize, &job->event, tempsize);
#endif
      filesize += tempsize;
      saved_count++;
    }
    job = job->next;
    // reset watchdog only if it seems that everything is going right
    if (saved_count <= count)
    {
      wdt_kick();
    }
  }
#ifdef DEBUG_CRON
  debug_printf("cron: all jobs written with total size of %i\n", filesize);
#endif

#ifdef CRON_VFS_SUPPORT
  vfs_truncate(file, filesize);
  vfs_close(file);
#else
  eeprom_update_chksum();
#endif
  return saved_count;
}
#endif

void
cron_init(void)
{
  // very important: set the linked lists head and tail to zero
  head = 0;
  tail = 0;

  // do we want to have some test entries?
#ifdef CRON_SUPPORT_TEST
#ifdef DEBUG_CRON
  debug_printf("cron: add test entries\n");
#endif
  addcrontest();
#endif

  last_check = 0;

#ifdef CRON_PERSIST_SUPPORT
  // load cron jobs form VFS
  cron_load();
#endif
}


int16_t
cron_jobinsert_callback(int8_t minute, int8_t hour, int8_t day, int8_t month,
                        int8_t daysofweek, uint8_t repeat, int8_t position,
                        void (*handler) (void *), uint8_t extrasize,
                        void *extradata)
{
  // emcd set?
  if (!handler || (extrasize == 0 && extradata))
    return -1;

  // try to get ram space
  struct cron_event_linkedlist *newone =
    malloc(sizeof(struct cron_event_linkedlist) + extrasize);

  // no more ram available -> abort
  if (!newone)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: not enough ram!\n");
#endif
    return -1;
  }

  // create new entry
  memset(newone, 0, sizeof(struct cron_event_linkedlist) + extrasize);
  newone->event.cond.minute = minute;
  newone->event.cond.hour = hour;
  newone->event.cond.day = day;
  newone->event.cond.month = month;
  newone->event.cond.daysofweek = daysofweek;
#ifdef CRON_DEFAULT_UTC
  newone->event.use_utc = 1;
#endif
  newone->event.repeat = repeat;
  newone->event.cmd = CRON_JUMP;
  newone->event.extrasize = extrasize;
  newone->event.handler = handler;
  strncpy(&(newone->event.extradata), extradata, extrasize);
  return cron_insert(newone, position);
}

int16_t
cron_jobinsert_ecmd(int8_t minute, int8_t hour, int8_t day, int8_t month,
                    int8_t daysofweek, uint8_t repeat, int8_t position,
                    char *ecmd)
{
  if (!ecmd || ecmd[0] == 0)
    return -1;
  uint8_t ecmdsize = strlen(ecmd) + 1;

  // try to get ram space
  struct cron_event_linkedlist *newone =
    malloc(sizeof(struct cron_event_linkedlist) + ecmdsize);

  // no more ram available -> abort
  if (!newone)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: not enough ram!\n");
#endif
    return -1;
  }

  // create new entry
  memset(newone, 0, sizeof(struct cron_event_linkedlist) + ecmdsize);
  newone->event.cond.minute = minute;
  newone->event.cond.hour = hour;
  newone->event.cond.day = day;
  newone->event.cond.month = month;
  newone->event.cond.daysofweek = daysofweek;
#ifdef CRON_DEFAULT_UTC
  newone->event.use_utc = 1;
#endif
  newone->event.repeat = repeat;
  newone->event.cmd = CRON_ECMD;
  newone->event.extrasize = ecmdsize;
  strncpy(&(newone->event.ecmddata), ecmd, ecmdsize);
  return cron_insert(newone, position);
}

uint8_t
cron_insert(struct cron_event_linkedlist * newone, int8_t position)
{
  // add to linked list
  if (!head)
  {                             // special case: empty list (ignore position)
    newone->prev = 0;
    newone->next = 0;
    head = newone;
    tail = newone;
#ifdef DEBUG_CRON
    debug_printf("cron: insert head\n");
#endif
    return 0;
  }

  if (position > 0)
  {
    uint8_t ss = 0;
    struct cron_event_linkedlist *job = head;

    // jump to position
    while (job)
    {
      if (ss++ == position)
        break;
      job = job->next;
    }

    newone->prev = job->prev;
    job->prev = newone;
    newone->next = job;
    if (job == head)
      head = newone;
#ifdef DEBUG_CRON
    if (newone->event.cmd == CRON_JUMP)
      debug_printf("cron: insert at %i jump \n", ss);
    else
      debug_printf("cron: insert at %i ecmd %s\n", ss,
                   newone->event.ecmddata);
#endif
    return ss;
  }

  // insert as last element
  newone->next = 0;
  newone->prev = tail;
  tail->next = newone;
  tail = newone;
#ifdef DEBUG_CRON
  debug_printf("cron: append\n");
#endif
  return cron_jobs() - 1;
}

void
cron_jobrm(struct cron_event_linkedlist *job)
{
  // null check
  if (!job)
    return;

  // remove link from element before this
  if (job == head)
    head = job->next;
  if (job->prev)
    job->prev->next = job->next;

  // remove link from element after this
  if (job == tail)
    tail = job->prev;
  if (job->next)
    job->next->prev = job->prev;

  // free the current element
  free(job);

#ifdef DEBUG_CRON
  debug_printf("cron: removed. Left %u\n", cron_jobs());
#endif
}

uint8_t
cron_jobs()
{
  uint8_t ss = 0;
  // count remaining elements
  struct cron_event_linkedlist *job = head;
  while (job)
  {
    ++ss;
    job = job->next;
  }
  return ss;
}


struct cron_event_linkedlist *
cron_getjob(uint8_t jobposition)
{
  // count remaining elements
  struct cron_event_linkedlist *job = head;
  while (job)
  {
    if (!jobposition)
      break;
    --jobposition;
    job = job->next;
  }

  if (jobposition)
    return 0;
  else
    return job;
}

void
cron_execute(struct cron_event_linkedlist *exec)
{
  if (exec->event.cmd == CRON_JUMP)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: match (JUMP %p)\n", &(exec->event.handler));
#endif
#ifndef DEBUG_CRON_DRYRUN
    exec->event.handler(&(exec->event.extradata));
#endif
  }
  else if (exec->event.cmd == CRON_ECMD)
  {
    // ECMD PARSER
#ifdef DEBUG_CRON
    debug_printf("cron: match (%s)\n", (char *) &(exec->event.ecmddata));
#endif
#ifndef DEBUG_CRON_DRYRUN
    char output[ECMD_INPUTBUF_LENGTH];
#ifdef DEBUG_CRON
    int16_t l =
#endif
      ecmd_parse_command((char *) &(exec->event.ecmddata), output,
                         sizeof(output) - 1);
#ifdef DEBUG_CRON
    if (is_ECMD_AGAIN(l))
      l = ECMD_AGAIN(l);
    if (is_ECMD_FINAL(l))
    {
      output[l] = 0;
      debug_printf("cron output %s\n", output);
    }
    else
    {
      debug_printf("cron output error %d\n", l);
    }
#endif
#endif
  }

  /* Execute job endless if repeat value is equal to zero otherwise
   * decrement the value and check if is equal to zero.
   * If that is the case, it is time to kick out this cronjob. */
  if (exec->event.repeat > 0 && !(--exec->event.repeat))
    cron_jobrm(exec);
}

#ifdef CRON_ANACRON_SUPPORT
uint8_t
cron_anacron(uint32_t starttime, uint32_t endtime)
{
  clock_datetime_t d, ld;
  struct cron_event_linkedlist *curr;

  /* count anacron jobs and set pending */
  uint8_t count = 0;
  for (curr = head; curr != 0; curr = curr->next)
  {
    if ((curr->event.anacron_pending = curr->event.anacron))
      count++;
  }
#ifdef DEBUG_CRON
  debug_printf("cron: %i anacron jobs found\n", count);
#endif
  if (count == 0)
    return 0;

  /* alloc space for anacron list */
  struct cron_event_linkedlist **tab =
    __builtin_alloca(count * sizeof(struct cron_event_linkedlist *));
  if (!tab)
  {
#ifdef DEBUG_CRON
    debug_printf("cron: not enough ram!\n");
#endif
    return 0;
  }

  /* limit range */
  if ((endtime - starttime) > CRON_ANACRON_MAXAGE)
    starttime = endtime - CRON_ANACRON_MAXAGE;

  /* prepare anacron tab (reverse time order!!) */
  uint8_t pos = 0;
  uint32_t timestamp;
  for (timestamp = endtime; timestamp > starttime && pos < count;
       timestamp -= 60)
  {
    clock_datetime(&d, timestamp);
    clock_localtime(&ld, timestamp);

    for (curr = head; curr != 0; curr = curr->next)
    {
      if (curr->event.anacron_pending &&
          cron_check_event(&curr->event.cond, curr->event.use_utc, &d, &ld))
      {
        curr->event.anacron_pending = 0;
        tab[pos++] = curr;
      }
    }
  }
#ifdef DEBUG_CRON
  debug_printf("cron: %i pending anacron jobs\n", pos);
#endif

  /* process jobs */
  while (pos)
    cron_execute(tab[--pos]);

  return 1;
}
#endif

void
cron_periodic(void)
{
  clock_datetime_t d, ld;
  uint32_t timestamp = clock_get_time();

  /* fix last_check */
  if (timestamp < last_check)
  {
    clock_datetime(&d, timestamp);
    last_check = timestamp - d.sec;
    return;
  }

  /* Check tasks at most once in a minute and only if at least one exists */
  if (!head || (timestamp - last_check) < 60)
    return;

  /* get time and date from unix timestamp */
  clock_datetime(&d, timestamp);
  clock_localtime(&ld, timestamp);

  /* truncate secs */
  timestamp -= d.sec;

#ifdef CRON_ANACRON_SUPPORT
  uint8_t skip_anacron = 0;
  if ((timestamp - last_check) > 60)
    skip_anacron = cron_anacron(last_check, timestamp);
#endif

  /* check every event for a match */
  struct cron_event_linkedlist *current = head;
  struct cron_event_linkedlist *exec;
  while (current)
  {
    /* backup current cronjob and advance current */
    exec = current;
    current = current->next;

#ifdef CRON_ANACRON_SUPPORT
    if (skip_anacron && exec->event.anacron)
      continue;
#endif
    /* if it matches all conditions , execute the handler function */
    if (cron_check_event(&exec->event.cond, exec->event.use_utc, &d, &ld))
      cron_execute(exec);
  }

  /* save the actual timestamp */
  last_check = timestamp;
}

/*
  -- Ethersex META --
  header(services/cron/cron.h)
  timer(50, cron_periodic())
  initearly(cron_init)
*/
