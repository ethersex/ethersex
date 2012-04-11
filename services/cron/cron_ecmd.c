/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "cron.h"

#include "protocols/ecmd/ecmd-base.h"

typedef struct
{
  uint8_t magic;
  uint8_t jobposition;
  struct cron_event_linkedlist *pos;
  uint8_t lineno;
} cron_list_state_t;

int16_t
parse_cmd_cron_list(char *cmd, char *output, uint16_t len)
{
  // trick: use bytes on cmd as "connection specific static variables"
  cron_list_state_t *state = (cron_list_state_t *) cmd;
  if (state->magic != 23)       // indicator flag: real invocation:  0
  {
    state->magic = 23;          // continuing call: 23
    state->jobposition = 0;
    state->pos = head;
    state->lineno = 0;
  }

  if (!state->pos)
    return ECMD_FINAL_OK;
  struct cron_event *job = &(state->pos->event);

  // first line: print job attributes
  if (state->lineno == 0)
  {
    state->lineno++;
    char *type;
    switch (job->cmd)
    {
      case CRON_JUMP:
        type = " jump";
        break;
      case CRON_ECMD:
        type = " ecmd";
        break;
      default:
        type = "";
    }
    return ECMD_AGAIN(snprintf_P(output, len, PSTR("cron %i rep %d%s%s"
#ifdef CRON_PERSIST_SUPPORT
                                                   "%s"
#endif
#ifdef CRON_ANACRON_SUPPORT
                                                   "%s"
#endif
                                 ),
                                 state->jobposition,
                                 job->repeat,
                                 type, (job->use_utc) ? " utc" : ""
#ifdef CRON_PERSIST_SUPPORT
                                 , (job->persistent) ? " pers" : ""
#endif
#ifdef CRON_ANACRON_SUPPORT
                                 , (job->anacron) ? " anac" : ""
#endif
                      ));
  }

  // print job
  int16_t ret;
  switch (job->cmd)
  {
    case CRON_JUMP:
      ret = snprintf_P(output, len, PSTR("%i %i %i %i %i %p"),
                       job->cond.minute, job->cond.hour, job->cond.day,
                       job->cond.month, job->cond.daysofweek, job->handler);
      break;
    case CRON_ECMD:
      ret = snprintf_P(output, len, PSTR("%i %i %i %i %i %s"),
                       job->cond.minute, job->cond.hour, job->cond.day,
                       job->cond.month, job->cond.daysofweek,
                       &(job->ecmddata));
      break;
    default:
      ret = snprintf_P(output, len, PSTR("%i %i %i %i %i"),
                       job->cond.minute, job->cond.hour, job->cond.day,
                       job->cond.month, job->cond.daysofweek);
  }

  state->jobposition++;
  state->pos = state->pos->next;
  state->lineno = 0;
  return ECMD_AGAIN(ret);
}

#ifdef CRON_PERSIST_SUPPORT
int16_t
parse_cmd_cron_save(char *cmd, char *output, uint16_t len)
{
  int16_t ret = cron_save();
  if (ret < 0)
    return ECMD_FINAL(snprintf_P(output, len, "write error"));

  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d jobs saved"), ret));
}

uint16_t
parse_cmd_cron_persistent(char *cmd, char *output, uint16_t len)
{
  uint8_t jobposition, state;
  uint8_t ret = sscanf_P(cmd, PSTR("%hhu %hhu"), &jobposition, &state);

  struct cron_event_linkedlist *jobll = NULL;
  if (ret >= 1)
  {
    jobll = cron_getjob(jobposition);
  }

  if (jobll == NULL)
    return ECMD_ERR_PARSE_ERROR;
  struct cron_event *job = &(jobll->event);

  if (ret >= 2)
  {
    job->persistent = state;
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("cron %d persistent %d"),
                               jobposition, job->persistent));
}
#endif

#ifdef CRON_ANACRON_SUPPORT
uint16_t
parse_cmd_cron_anacron(char *cmd, char *output, uint16_t len)
{
  uint8_t jobposition, state;
  uint8_t ret = sscanf_P(cmd, PSTR("%hhu %hhu"), &jobposition, &state);

  struct cron_event_linkedlist *jobll = NULL;
  if (ret >= 1)
  {
    jobll = cron_getjob(jobposition);
  }

  if (!jobll)
    return ECMD_ERR_PARSE_ERROR;
  struct cron_event *job = &(jobll->event);

  if (ret >= 2)
  {
    job->anacron = state;
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("cron %d anacron %d"),
                               jobposition, job->anacron));
}
#endif

uint16_t
parse_cmd_cron_utc(char *cmd, char *output, uint16_t len)
{
  uint8_t jobposition, state;
  uint8_t ret = sscanf_P(cmd, PSTR("%hhu %hhu"), &jobposition, &state);

  struct cron_event_linkedlist *jobll = NULL;
  if (ret >= 1)
  {
    jobll = cron_getjob(jobposition);
  }

  if (!jobll)
    return ECMD_ERR_PARSE_ERROR;
  struct cron_event *job = &(jobll->event);

  if (ret >= 2)
  {
    job->use_utc = state;
    return ECMD_FINAL_OK;
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("cron %d utc %d"),
                               jobposition, job->use_utc));
}

int16_t
parse_cmd_cron_rm(char *cmd, char *output, uint16_t len)
{
  uint8_t jobposition;
  uint8_t ret = sscanf_P(cmd, PSTR("%hhu"), &jobposition);

  if (ret == 1)
  {
    struct cron_event_linkedlist *jobll = cron_getjob(jobposition);
    if (!jobll)
      return ECMD_ERR_PARSE_ERROR;
    cron_jobrm(jobll);
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("rm cron %u"), jobposition));
  }

  while (head)
    cron_jobrm(head);
  return ECMD_FINAL(snprintf_P(output, len, PSTR("rm all cron")));
}

// generic cron add
// Fields: Min Hour Day Month Dow ecmd
int16_t
parse_cmd_cron_add(char *cmd, char *output, uint16_t len)
{
  int8_t minute, hour, day, month, dayofweek;
  int i;

  if (sscanf_P
      (cmd, PSTR("%hhi %hhi %hhi %hhi %hhi %n"), &minute, &hour, &day, &month,
       &dayofweek, &i) != 5)
  {
    return ECMD_ERR_PARSE_ERROR;
  }

  int16_t ret =
    cron_jobinsert_ecmd(minute, hour, day, month, dayofweek, INFINIT_RUNNING,
                        CRON_APPEND, cmd + i);
  if (ret < 0)
  {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("add cron failed")));
  }

  return ECMD_FINAL(snprintf_P(output, len, PSTR("add cron %d"), ret));
}

/*
  -- Ethersex META --
  block([[CRON-Dienst]])
  ecmd_feature(cron_list, "cron list",, Show all cron entries)
ecmd_ifdef(CRON_PERSIST_SUPPORT)
  ecmd_feature(cron_save, "cron save",, Saves all persistent jobs)
  ecmd_feature(cron_persistent, "cron persistent", POSITION [STATE], show/set job persistance state)
ecmd_endif()
ecmd_ifdef(CRON_ANACRON_SUPPORT)
  ecmd_feature(cron_anacron, "cron anacron", POSITION [STATE], show/set job anacron state)
ecmd_endif()
  ecmd_feature(cron_utc, "cron utc", POSITION [STATE], show/set utc state)
  ecmd_feature(cron_rm, "cron rm", [POSITION], Remove one cron entry)
  ecmd_feature(cron_add, "cron add", MIN HOUR DAY MONTH DOW ECMD, Add ECMD to cron to be executed at given time)
*/
