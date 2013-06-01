/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _SGC_H
#define _SGC_H
#include "protocols/ecmd/via_tcp/ecmd_state.h"
#define SGC_BUFFER_LENGTH ECMD_INPUTBUF_LENGTH - 12 + 6
/* min string ECMD length: 12 (sgc_stt) */
/* SGC command length is 6 */

struct sgc_buffer
{
  uint8_t len;
  uint8_t pos;
  char txdata[SGC_BUFFER_LENGTH];
};

struct sgc_vars
{
  char contrast;
  char pensize;
  char font;
  char opacity;
  char colour[2];
};

struct sgc_state
{
  uint8_t ist;
  uint8_t ack;
  uint8_t timer;
  uint8_t baudcounter;
  uint16_t acktimer;
  uint16_t ack_timeout;

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
  uint16_t mincount;
  uint8_t timeout;
  uint8_t timer_max;
#endif                          /* SGC_TIMEOUT_COUNTER_SUPPORT */
};

/* define bitstates */
#define SH_SLEEP 0
#define SH_SLEEPING 1
#define SH_F_RESET 2
#define SH_TXBUSY 3
#define SH_RXENABLE 4
#define SH_ACK_INF 5
#define SLEEP (0x01 << SH_SLEEP)
#define SLEEPING (0x01 << SH_SLEEPING)
#define F_RESET (0x01 << SH_F_RESET)
#define TXBUSY (0x01 << SH_TXBUSY)
#define RXENABLE (0x01 << SH_RXENABLE)
#define ACK_INF (0x01 << SH_ACK_INF)

/* define some important states */
#define DISP_RESET 0
#define BEGIN_SHUTDOWN 5
#define SHUTDOWN 10
#define BEGIN_POWERUP 11
#define POWERUP 18

/* define display answers */
#define FROM_RESET 0
#define ACK 1
#define NACK 2
#define SENDING 3
#define WAKEUP 4
#define NONE 5
#define BUSY 6
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
#define HALF_SEC 25
#define TWO_SEC 100
#define FIVE_SEC 250
#define INF_TIMER TWO_SEC

void sgc_init(void);
uint8_t sgc_setpowerstate(uint8_t soll);
uint8_t sgc_getpowerstate(void);
uint8_t sgc_sleep(char mode);

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
