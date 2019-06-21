/*
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012-2019 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

/* httplog() can be used to log an arbitrary string against a web server
 * optionally including the current unix time stamp and a unique machine
 * identifier (uuid) thus providing a similar facility as syslog
 */

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"
#ifdef DEBUG_HTTPLOG
#include "core/debug.h"
#endif
#include "core/queue/queue.h"
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
#include "services/clock/clock.h"
#endif
#include "httplog.h"

#ifdef DEBUG_HTTPLOG
#define HTTPLOG_DEBUG(a...)  debug_printf("httplog: " a)
#else
#define HTTPLOG_DEBUG(a...)  do { } while(0)
#endif


static Queue httplog_queue = {.limit = HTTPLOG_QUEUE_LEN };

static uint8_t request_pending;

// first string is the GET part including the path
static const char PROGMEM get_string_head[] = "GET " CONF_HTTPLOG_PATH "?";
// next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_HTTPLOG_INCLUDE_UUID
static const char PROGMEM uuid_string[] = "uuid=" CONF_HTTPLOG_UUID "&";
#endif
// and the http footer including the http protocol version and the server name
static const char PROGMEM get_string_foot[] =
  " HTTP/1.1\n" "Host: " CONF_HTTPLOG_SERVICE "\r\n\r\n";


static void
httplog_net_main(void)
{
  HTTPLOG_DEBUG("uip_flags=0x%02x\n", uip_flags);

  if (uip_aborted() || uip_timedout())
  {
    HTTPLOG_DEBUG("Connection aborted.\n");
    request_pending = 0;
    return;
  }

  else if (uip_closed())
  {
    HTTPLOG_DEBUG("Connection closed.\n");
    request_pending = 0;
    return;
  }

  else if (uip_connected() || uip_rexmit())
  {
    HTTPLOG_DEBUG("%S\n", uip_connected()? PSTR("New connection.")
                                         : PSTR("Rexmit."));

    char *data = peek(&httplog_queue);
    if (data == NULL)
    {
      HTTPLOG_DEBUG("Queue is empty!\n");
      uip_close();
    }
    else
    {
#define BUFFER_AVAIL (size_t)(UIP_APPDATA_SIZE-(p-(char*)uip_appdata))
      char *p = uip_appdata;
      p += snprintf_P(p, BUFFER_AVAIL, get_string_head);
#ifdef CONF_HTTPLOG_INCLUDE_UUID
      p += snprintf_P(p, BUFFER_AVAIL, uuid_string);
#endif
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
      p += snprintf_P(p, BUFFER_AVAIL, PSTR("time=%lu&"), clock_get_time());
#endif
      p += snprintf_P(p, BUFFER_AVAIL, PSTR("%s%S"), data, get_string_foot);
      if (BUFFER_AVAIL <= 0)
        HTTPLOG_DEBUG("Message truncated!\n");
      uip_udp_send(p - (char *) uip_appdata);
      HTTPLOG_DEBUG("Sending %d bytes.\n", p - (char *) uip_appdata);
    }
  }

  else if (uip_acked())
  {
    HTTPLOG_DEBUG("Acked.\n");
    uip_close();
    free(pop(&httplog_queue));  // delete data only when acked
  }
}

static void
httplog_connect(char *hostname, uip_ipaddr_t * ipaddr)
{
  if (ipaddr != NULL)
  {
    HTTPLOG_DEBUG("Connecting to %s (%d.%d.%d.%d).\n",
                  hostname,
                  ((unsigned char *) ipaddr)[0],
                  ((unsigned char *) ipaddr)[1],
                  ((unsigned char *) ipaddr)[2],
                  ((unsigned char *) ipaddr)[3]);
    if (uip_connect(ipaddr, HTONS(80), httplog_net_main) != NULL)
      return;
    else
      HTTPLOG_DEBUG("Connect failed.\n");
  }
  else
    HTTPLOG_DEBUG("Resolve failed!\n");

  request_pending = 0;          // retried later via httplog_flush
}

static void
httplog_transmit(void)
{
  request_pending = 1;

  uip_ipaddr_t *ipaddr;
  if ((ipaddr = resolv_lookup(CONF_HTTPLOG_SERVICE)) == NULL)
  {
    HTTPLOG_DEBUG("Resolving address of %s.\n", CONF_HTTPLOG_SERVICE);
    resolv_query(CONF_HTTPLOG_SERVICE, httplog_connect);
  }
  else
  {
    httplog_connect((char *) CONF_HTTPLOG_SERVICE, ipaddr);
  }
}

static uint8_t
httplog_enqueue(char *data)
{
  uint8_t result = push(data, &httplog_queue);
  if (!result)
    free(data);
  return result;
}

uint8_t
httplog(const char *message, ...)
{
  va_list va;
  va_start(va, message);
  size_t len = (size_t) vsnprintf(NULL, 0, message, va);
  va_end(va);

  char *data = malloc(len);
  if (data == NULL)
    return 0;

  va_start(va, message);
  vsnprintf(data, len, message, va);
  va_end(va);

  return httplog_enqueue(data);
}

uint8_t
httplog_P(const char *message, ...)
{
  va_list va;
  va_start(va, message);
  size_t len = (size_t) vsnprintf_P(NULL, 0, message, va);
  va_end(va);

  char *data = malloc(len);
  if (data == NULL)
    return 0;

  va_start(va, message);
  vsnprintf_P(data, len, message, va);
  va_end(va);

  return httplog_enqueue(data);
}

void
httplog_flush(void)
{
  //HTTPLOG_DEBUG("%u entries in queue.\n", httplog_queue.count);

  if (isEmpty(&httplog_queue))
    return;                     // nothing to send
  if (request_pending)
    return;                     // previous request not finished
  httplog_transmit();
}

/*
  -- Ethersex META --
  header(protocols/httplog/httplog.h)
  mainloop(httplog_flush)
*/
