/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
 * Copyright (c) 2019 by Erik Kunze <ethersex@erik-kunze.de>
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
#include <stdlib.h>
#include <string.h>

#include "protocols/uip/uip.h"
#include "config.h"
#include "core/debug.h"
#include "core/param.h"
#include "core/queue/queue.h"
#include "protocols/uip/uip_router.h"
#include "protocols/uip/check_cache.h"
#include "syslog.h"
#include "syslog_net.h"

#define UIP_MAX_LENGTH (UIP_BUFSIZE - UIP_IPUDPH_LEN - UIP_LLH_LEN)

extern uip_udp_conn_t *syslog_conn;
static Queue syslog_queue = {.limit = SYSLOG_QUEUE_LEN };


static uint8_t
syslog_enqueue(char *data)
{
  uint8_t result = queue_push(data, &syslog_queue);
  if (!result)
    free(data);
  return result;
}

uint8_t
syslog_send(const char *message)
{
  size_t len = strlen(message);
  if (len == 0)
    return 1;                   /* zero sized message -> pretend it was sent */

  len = MIN(len, UIP_MAX_LENGTH);

  char *data = malloc(len + 1);
  if (data == NULL)
    return 0;

  strncpy(data, message, len);
  data[len] = 0;

  return syslog_enqueue(data);
}

uint8_t
syslog_sendf_P(const char *message, ...)
{
  va_list va;
  va_start(va, message);
  size_t len = (size_t) vsnprintf_P(NULL, 0, message, va);
  va_end(va);

  if (len == 0)
    return 1;                   /* zero sized message -> pretend it was sent */

  len = MIN(len, UIP_MAX_LENGTH) + 1;
  char *data = malloc(len);
  if (data == NULL)
    return 0;

  va_start(va, message);
  vsnprintf_P(data, len, message, va);
  va_end(va);

  return syslog_enqueue(data);
}

void
syslog_flush(void)
{
#ifdef ETHERNET_SUPPORT
  if (!syslog_conn || uip_check_cache(&syslog_conn->ripaddr))
    return;                     /* ARP cache not ready, don't send request
                                 * here (would flood, wait for poll event). */
#endif /* ETHERNET_SUPPORT */

  if (queue_is_empty(&syslog_queue))
    return;

  uip_appdata = uip_sappdata = &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
  uip_slen = 0;

  char *data = queue_pop(&syslog_queue);
  size_t len = strlen(data);

  /* The string truncation in syslog_send/syslog_send_P guarantees that
   * memcpy never writes over the end of the destination buffer. */
  memcpy(uip_appdata, data, len);
  free(data);
  uip_udp_send((int) len);

  uip_udp_conn = syslog_conn;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}

/*
  -- Ethersex META --
  header(protocols/syslog/syslog.h)
  mainloop(syslog_flush)
*/
