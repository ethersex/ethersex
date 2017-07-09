/*
 * Copyright (c) 2013-2017 Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "ecmd_bt.h"
#include "hardware/radio/bluetooth/bt_usart.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"
#include "ecmd_bt.h"


static char recv_buffer[ECMD_SERIAL_USART_BUFFER_LEN];
static char write_buffer[ECMD_SERIAL_USART_BUFFER_LEN + 2];
static uint8_t recv_len, sent;
static int16_t write_len;
static volatile uint8_t must_parse;

void
ecmd_bt_periodic(void)
{
  if (!bt_init_finished)
    return;

  if (must_parse && write_len == 0)
  {
    /* we have a request */
    must_parse = 0;

    if (recv_len <= 1)
    {
      recv_len = 0;
      return;
    }

    write_len = ecmd_parse_command(recv_buffer,
                                   write_buffer, sizeof(write_buffer));
    if (is_ECMD_AGAIN(write_len))
    {
      /* convert ECMD_AGAIN back to ECMD_FINAL */
      write_len = ECMD_AGAIN(write_len);
      must_parse = 1;
    }
    else if (is_ECMD_ERR(write_len))
    {
      return;
    }
    else
    {
      recv_len = 0;
    }

    write_buffer[write_len++] = '\r';
    write_buffer[write_len++] = '\n';

    sent = 1;
    bt_usart_putchar(write_buffer[0]);
  }
}


void
ecmd_bt_recv(void)
{
  while (bt_usart_avail())
  {
    if (must_parse)
      return;

    uint8_t data = bt_usart_getchar();

    if (data == '\n' || data == '\r' || recv_len == sizeof(recv_buffer))
    {
      recv_buffer[recv_len] = 0;
      must_parse = 1;
      bt_usart_putchar('\r');
      bt_usart_putchar('\n');
      return;
    }

#ifndef ECMD_SERIAL_NO_ECHO
    bt_usart_putchar(data);
#endif

    recv_buffer[recv_len++] = data;
  }
}


void
ecmd_bt_send(void)
{
  while (sent < write_len && bt_usart_free())
  {
    bt_usart_putchar(write_buffer[sent++]);
  }
  if (sent >= write_len)
    write_len = 0;
}


/*
  -- Ethersex META --
  header(protocols/ecmd/via_bluetooth/ecmd_bt.h)
  timer(1,ecmd_bt_periodic())
*/
