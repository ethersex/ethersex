/*
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SGC_H
#define _SGC_H
#include "protocols/ecmd/via_tcp/ecmd_state.h"
#define SGC_BUFFER_LENGTH ECMD_INPUTBUF_LENGTH - 12 + 6
/* min string ECMD length: 12 (sgc_stt) */
/* SGC command length is 6 */

struct sgc_buffer
{
  uint8_t len;                  /* command length */
  uint8_t pos;                  /* TX pointer */
  char txdata[SGC_BUFFER_LENGTH];       /* TX buffer */
};

struct sgc_vars                 /* storage for display settings */
{
  char contrast;
  char pensize;
  char font;
  char opacity;
  char colour[2];
};

struct sgc_state
{
  uint8_t ist;                  /* current state */
  uint8_t ack;                  /* command status */
  uint8_t counter;              /* timer for reset / autobaud */
  uint16_t acktimer;            /* timer for display answer */
  uint16_t ack_timeout;         /* max. time for display answer */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  uint16_t mincount;            /* one-minute-timer */
  uint8_t timeout;              /* no-activity time counter */
  uint8_t timer_max;            /* no-activity timeout value */
#endif                          /* SGC_TIMEOUT_COUNTER_SUPPORT */
};

/* define display answers */
#define SGC_FROM_RESET 0
#define SGC_ACK 1
#define SGC_NACK 2
#define SGC_SENDING 3
#define SGC_WAKEUP 4
#define SGC_NONE 5
#define SGC_BUSY 6
#define SGC_TIMEOUT 7

/* define command options */
#define SGC_SH_STRING 0
#define SGC_SH_NORMAL 1
#define SGC_SH_INTERNAL 2
#define SGC_OPT_STRING (0x01 << SGC_SH_STRING)
#define SGC_OPT_NORMAL (0x01 << SGC_SH_NORMAL)
#define SGC_OPT_INTERNAL (0x01 << SGC_SH_INTERNAL)

/* define ACK waiting times */
#define SGC_INFINITE 0
#define SGC_ONE_SEC 50
#define SGC_FIVE_SEC 250

/* define some important states */
#define SGC_DISP_RESET 0
#define SGC_BEGIN_SHUTDOWN 5
#define SGC_SHUTDOWN 11
#define SGC_BEGIN_POWERUP 12
#define SGC_POWERUP 19

void sgc_init(void);
uint8_t sgc_setpowerstate(uint8_t soll);
uint8_t sgc_getpowerstate(void);
void sgc_setshdnsleep(char sleepmode);
uint8_t sgc_sleep(char mode, uint8_t option);

#ifdef SGC_ECMD_SEND_SUPPORT
int8_t sgc_setip(char *data);
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
void sgc_settimeout(uint8_t time);
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

uint8_t sgc_sendcommand(uint8_t cmdlen, uint16_t timeout, uint8_t option,
                        char *data, char *cmdline);
uint8_t sgc_getcommandresult(void);
uint8_t sgc_setcontrast(char contrast);
uint8_t sgc_setpensize(char pensize);
uint8_t sgc_setfont(char font);
char sgc_getfont(void);
uint8_t sgc_setopacity(char opacity);
void sgc_setcolour(char *colour);
void sgc_getcolour(char *colour);
uint8_t rgb2sgc(char *col, int8_t stop);
void sgc_pwr_periodic(void);

#endif /* _SGC_H */
