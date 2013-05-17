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

#ifndef _SGC_H
#define _SGC_H
#define SGC_BUFFER_LENGTH 17

struct sgc_buffer {
  uint8_t len;
  uint8_t pos;
  uint8_t busy;
  uint8_t rxenable;
  uint8_t txdata[SGC_BUFFER_LENGTH];
  uint8_t rxdata;
  uint8_t ack;
  uint8_t acktimer;
};

struct sgc_state {
 uint8_t ist;
 uint8_t contrast;
 uint8_t timer;
 uint8_t baudcounter;
 uint8_t from_reset;
#ifdef SGC_TIMEOUT_COUNTER
 uint16_t mincount;
 uint8_t timeout;
#endif /* SGC_TIMEOUT_COUNTER */
};

/* define some important states */
#define DISP_RESET 0
#define SHUTDOWN 9
#define POWERUP 14
#define BEGIN_SHUTDOWN 5
#define BEGIN_POWERUP 10

/* define display answers */
#define FROM_RESET 0
#define ACK 1
#define NACK 2
#define SENDING 3
#define NONE 4
#define BUSY 5
#define TIMEOUT 6

void sgc_init (void);
uint8_t sgc_setpowerstate (uint8_t soll);
uint8_t sgc_getpowerstate (void);
uint8_t sgc_sendcommand (uint8_t cmdlen, uint8_t *data, uint8_t from_sequence);
uint8_t sgc_getcommandresult (void);
uint8_t sgc_setcontrast (uint8_t contrast);
uint8_t rgb2sgc (uint8_t red, uint8_t green, uint8_t blue, uint8_t *ret);
#ifdef SGC_TIMEOUT_COUNTER
void sgc_reset_timeout (void);
#endif /* SGC_TIMEOUT_COUNTER */
void sgc_pwr_periodic (void);

extern struct sgc_buffer sgc_send;
extern struct sgc_state sgc_power_state;

#endif /* _SGC_H */
