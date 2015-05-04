/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
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

#include <avr/pgmspace.h>
#include <stdarg.h>

#include "protocols/uip/uip.h"
#include "config.h"
#include "core/debug.h"
#include "protocols/uip/uip_neighbor.h"
#include "protocols/uip/uip_router.h"
#include "protocols/uip/check_cache.h"
#include "syslog.h"
#include "syslog_net.h"


static char send_buffer[MAX_LINES_SYSLOG_BUFFER][MAX_LENGTH_SYSLOG_BUFFER + 1];
extern uip_udp_conn_t *syslog_conn;
static struct SyslogCallbackCtx syslog_callbacks[SYSLOG_CALLBACKS];
int8_t act_char_line = -1;

static void
syslog_send_cb_P(void *data)
{
  strcpy_P(uip_appdata, data);
  uip_udp_send(strlen_P(data));
}

static void
syslog_send_cb(void *data)
{
  char *p = data;

  strcpy(uip_appdata, p);
  uip_udp_send(strlen(p));

  for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
  {
    if (p == send_buffer[i])
      p[0] = 0;
    if (i == act_char_line)
      act_char_line = -1;
  }
}

uint8_t
syslog_send_P(PGM_P message)
{
  return syslog_insert_callback(syslog_send_cb_P, (void *) message);
}

uint8_t
syslog_send(const char *message)
{
  for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
    if (send_buffer[i][0] == 0)
    {
      if (strlen(message) + 1 > MAX_LENGTH_SYSLOG_BUFFER)
        return 0;

      strncpy(send_buffer[i], message, strlen(message) + 1);

      return syslog_insert_callback(syslog_send_cb, (void *) send_buffer[i]);
    }
  return 1;
}

uint8_t
syslog_send_char(const char c)
{
  if (act_char_line < 0)
    // Search free send_buffer
    for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
      if (send_buffer[i][0] == 0)
        act_char_line = i;

  if (act_char_line >= 0)
  {
    uint16_t offset = strlen(send_buffer[act_char_line]);

    if (offset + 2 > MAX_LENGTH_SYSLOG_BUFFER)
      return 0;

    send_buffer[act_char_line][offset] = c;
    send_buffer[act_char_line][offset + 1] = 0;

    if (!offset)
      /* If there used to be a few bytes in the buffer already, we don't have
         to insert the callback, since it should have been added already. */
      if (syslog_insert_callback
          (syslog_send_cb, (void *) send_buffer[act_char_line]))
        return 1;

    if (c == '\n')              // Newline start new message on next char
      act_char_line = -1;

    return 0;
  }
  return 1;
}

uint8_t
syslog_sendf(const char *message, ...)
{
  va_list va;
  for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
    if (send_buffer[i][0] == 0)
    {

      va_start(va, message);
      vsnprintf(send_buffer[i], MAX_LENGTH_SYSLOG_BUFFER, message, va);
      va_end(va);

      send_buffer[i][MAX_LENGTH_SYSLOG_BUFFER] = 0;

      return syslog_insert_callback(syslog_send_cb, (void *) send_buffer[i]);
    }
  return 1;
}

uint8_t
syslog_sendf_P(PGM_P message, ...)
{
  va_list va;
  for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
    if (send_buffer[i][0] == 0)
    {

      va_start(va, message);
      vsnprintf_P(send_buffer[i], MAX_LENGTH_SYSLOG_BUFFER, message, va);
      va_end(va);

      send_buffer[i][MAX_LENGTH_SYSLOG_BUFFER] = 0;

      return syslog_insert_callback(syslog_send_cb, (void *) send_buffer[i]);
    }
  return 1;
}

uint8_t
syslog_send_ptr(void *message)
{
  return syslog_insert_callback(syslog_send_cb, message);
}


void
syslog_flush(void)
{
#ifdef ETHERNET_SUPPORT
  if (!syslog_conn || uip_check_cache(&syslog_conn->ripaddr))
    return;                     /* ARP cache not ready, don't send request
                                 * here (would flood, wait for poll event). */
#endif /* ETHERNET_SUPPORT */

  uip_slen = 0;
  uip_appdata = uip_sappdata = uip_buf + UIP_IPUDPH_LEN + UIP_LLH_LEN;

  for (uint8_t i = 0; i < SYSLOG_CALLBACKS; i++)
    if (syslog_callbacks[i].callback != NULL)
    {
      syslog_callbacks[i].callback(syslog_callbacks[i].data);
      syslog_callbacks[i].callback = NULL;
      break;
    }

  if (!uip_slen)
    return;

  uip_udp_conn = syslog_conn;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}

uint8_t
syslog_insert_callback(syslog_callback_t callback, void *data)
{
  uint8_t i;
  for (i = 0; i < SYSLOG_CALLBACKS; i++)
    if (syslog_callbacks[i].callback == NULL)
    {
      syslog_callbacks[i].callback = callback;
      syslog_callbacks[i].data = data;
      return 1;
    }
  return 0;                     /* No empty callback found */
}

void
syslog_init(void)
{
  for (uint8_t i = 0; i < MAX_LINES_SYSLOG_BUFFER; i++)
    send_buffer[i][0]=0;
}
/*
  -- Ethersex META --
  header(protocols/syslog/syslog.h)
  init(syslog_init)
  mainloop(syslog_flush)
*/
