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

/* define bitstates */
#define SH_SLEEP 0              /* go to sleep mode */
#define SH_SLEEPING 1           /* in sleep mode */
#define SH_F_RESET 2            /* coming from reset */
#define SH_TXBUSY 3             /* reserve TX for internal command */
#define SH_RXENABLE 4           /* enable RX after reset */
#define SH_SHDN_SLEEP 5         /* enable auto-sleep in shutdown */
#define SH_SHDN_SLEEPMODE (SH_SHDN_SLEEP + 1)   /* auto-sleep mode */
#define SLEEP (0x01 << SH_SLEEP)
#define SLEEPING (0x01 << SH_SLEEPING)
#define F_RESET (0x01 << SH_F_RESET)
#define TXBUSY (0x01 << SH_TXBUSY)
#define RXENABLE (0x01 << SH_RXENABLE)
#define SHDN_SLEEP (0x01 << SH_SHDN_SLEEP)
#define SHDN_SLEEPMODE (0x01 << SH_SHDN_SLEEPMODE)

/* define some important states */
#define DISP_RESET 0
#define BEGIN_SHUTDOWN 5
#define SHUTDOWN 11
#define BEGIN_POWERUP 12
#define POWERUP 19

/* define display answers */
#define FROM_RESET 0
#define ACK 1
#define NACK 2
#define SENDING 3
#define WAKEUP 4
#define NONE 5
#define SGC_BUSY 6
#define TIMEOUT 7

/* define command options */
#define SH_STRING 0
#define SH_NORMAL 1
#define SH_INTERNAL 2
#define OPT_STRING (0x01 << SH_STRING)
#define OPT_NORMAL (0x01 << SH_NORMAL)
#define OPT_INTERNAL (0x01 << SH_INTERNAL)

/* define ACK waiting times */
#define INFINITE 0
#define ONE_SEC 50
#define FIVE_SEC 250

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
