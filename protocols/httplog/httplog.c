/*
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
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
#include "protocols/uip/uip.h"
#include "protocols/dns/resolv.h"
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
#include "services/clock/clock.h"
#endif
#include "httplog.h"

#ifdef HTTPLOG_UUID_EEPROM
#include "core/eeprom.h"
#endif

#ifdef DEBUG_HTTPLOG
#define HTTPLOG_DEBUG(a...)  debug_printf("httplog: " a)
#else
#define HTTPLOG_DEBUG(a...)
#endif

static char *httplog_tmp_buf=NULL;
// httplog_tmp_buf is being cleared after ACK, connection close, connection abort. However if a new connection is created between ACK and abort it is cleared again and nothing is sent.
uip_conn_t *httpConn=NULL;

// first string is the GET part including the path
static const char PROGMEM get_string_head[] = "GET " CONF_HTTPLOG_PATH "?";
// and the http footer including the http protocol version and the server name
static const char PROGMEM get_string_foot[] =
  " HTTP/1.1\n" "Host: " CONF_HTTPLOG_SERVICE "\r\n\r\n";


static void
httplog_net_main(void)
{
  HTTPLOG_DEBUG("httplog_net_main[%u->%u] start\n",ntohs(uip_conn->lport), ntohs(uip_conn->rport));
  // we expect that aborted | timeout | closed is only triggered ONCE (otherwise httplog_tmp_buf of another message might be freed)
  if (uip_aborted() || uip_timedout() || uip_closed())
  {
    HTTPLOG_DEBUG("httplog_net_main[%u] connection aborted=%d, timedout=%d, closed=%d\n",ntohs(uip_conn->lport), uip_aborted(), uip_timedout(), uip_closed());
	// according to uip doc cleanup should be called in close(), however this is not called in case of timeout.
    if (httplog_tmp_buf)
    {
      free(httplog_tmp_buf);
      httplog_tmp_buf = NULL;
      HTTPLOG_DEBUG("httplog_net_main[%u] free httplog_tmp_buf\n",ntohs(uip_conn->lport));
    }
    goto end;
  }

/*
  if (uip_closed())
  {
    HTTPLOG_DEBUG("httplog_net_main[%u] connection closed\n",ntohs(uip_conn->lport));
    // uip docu: cleanup should be here
    goto end;
  }
  */


  if (uip_connected() || uip_rexmit())
  {
#define BUFFER_AVAIL UIP_APPDATA_SIZE - (p-(char*)uip_appdata)
    HTTPLOG_DEBUG("httplog_net_main[%u] new connection or rexmit, sending message %p=[%s], UIP_APPDATA_SIZE=%d\n",ntohs(uip_conn->lport),httplog_tmp_buf,httplog_tmp_buf,UIP_APPDATA_SIZE);
    char *p = uip_appdata;
    p += snprintf_P(p, BUFFER_AVAIL, get_string_head);
    // next is the - optional - inclusion of the machine identifier uuid
#ifdef CONF_HTTPLOG_INCLUDE_UUID
  #ifdef HTTPLOG_UUID_EEPROM
    char uuid_buffer[sizeof(CONF_HTTPLOG_UUID)+1];
	eeprom_restore(httplog_uuid, uuid_buffer, sizeof(uuid_buffer));
    p += snprintf_P(p, BUFFER_AVAIL, PSTR("uuid=%s&"), uuid_buffer);
  #else
    p += snprintf_P(p, BUFFER_AVAIL, PSTR("uuid=%S&"), PSTR(CONF_HTTPLOG_UUID));
  #endif
#endif
    // the - optional - unix time stamp
#ifdef CONF_HTTPLOG_INCLUDE_TIMESTAMP
    p += snprintf_P(p, BUFFER_AVAIL, PSTR("time=%lu&"), clock_get_time());
#endif
    p += snprintf_P(p, BUFFER_AVAIL, PSTR("%s%S"), httplog_tmp_buf, get_string_foot);
    if(BUFFER_AVAIL <= 0) {
      HTTPLOG_DEBUG("httplog_net_main[%u] WARN: buffer too short (%d)\n", BUFFER_AVAIL);
    }
    uip_udp_send(p - (char *) uip_appdata);
    HTTPLOG_DEBUG("httplog_net_main[%u] send %d bytes\n",ntohs(uip_conn->lport), p - (char *) uip_appdata);
  }

  if (uip_acked())
  {
    HTTPLOG_DEBUG("httplog_net_main[%u] acked\n",ntohs(uip_conn->lport));
    uip_close();
  end:
    /*
    if (httplog_tmp_buf)
    {
      free(httplog_tmp_buf);
      httplog_tmp_buf = NULL;
      HTTPLOG_DEBUG("httplog_net_main[%d] free httplog_tmp_buf\n",uip_conn->lport);
    }
	*/
    HTTPLOG_DEBUG("httplog_net_main[%u] end\n",ntohs(uip_conn->lport));
  }
}

static void
httplog_dns_query_cb(char *name, uip_ipaddr_t * ipaddr)
{
  HTTPLOG_DEBUG("got dns response, connecting\n");
  httpConn = uip_connect(ipaddr, HTONS(80), httplog_net_main);
  if (!httpConn)
  // if (!uip_connect(ipaddr, HTONS(80), httplog_net_main))
  {
    HTTPLOG_DEBUG("error\n");
    if (httplog_tmp_buf)
    {
      free(httplog_tmp_buf);
      httplog_tmp_buf = NULL;
    }
  }
  HTTPLOG_DEBUG("httplog_dns_query_cb done\n");

}

/**
 *
 * @return true if new buffer could be created
 */
uint8_t
httplog_buffer_empty(int len)
{
  if (httplog_tmp_buf == 0)
  {
    httplog_tmp_buf = malloc(HTTPLOG_BUFFER_LEN);
    if (httplog_tmp_buf != 0)
      return 1;
    HTTPLOG_DEBUG("httplog_buffer_empty malloc failed!\n");
  }

  return 0;
}

static void
httplog_resolve_address(void)
{
  HTTPLOG_DEBUG("httplog_resolve_address\n");
  uip_ipaddr_t *ipaddr;

  char conf_httplog_service[sizeof(CONF_HTTPLOG_SERVICE)];
  memcpy_P(conf_httplog_service, PSTR(CONF_HTTPLOG_SERVICE), sizeof(CONF_HTTPLOG_SERVICE));
  if (!(ipaddr = resolv_lookup(conf_httplog_service)))
  {
    HTTPLOG_DEBUG("httplog_resolve_address resolv_query\n");
    resolv_query(conf_httplog_service, httplog_dns_query_cb);
    HTTPLOG_DEBUG("httplog_resolve_address resolv_query done\n");
  }
  else
  {
    httplog_dns_query_cb(NULL, ipaddr);
  }
  HTTPLOG_DEBUG("httplog_resolve_address done\n");
}

uint8_t
httplog(const char *message, ...)
{
  if (httplog_tmp_buf == 0)
  {
    va_list va;
    va_start(va, message);
    int len=vsnprintf(NULL, 0, message, va);
    va_end(va);
    if(len >= HTTPLOG_BUFFER_LEN) {
      HTTPLOG_DEBUG("error: message too long %d\n", len);
      return 0;
    }
    uint8_t rc=httplog_buffer_empty(len+1);
    if(!rc) {
      HTTPLOG_DEBUG("error: allocating buffer\n");
    } else if(rc) {
      va_start(va, message);
      vsnprintf(httplog_tmp_buf, len+1, message, va);
      va_end(va);
      httplog_tmp_buf[len] = 0;
      HTTPLOG_DEBUG("httplog send message [%s]\n", httplog_tmp_buf);
      httplog_resolve_address();
      return 1;
    }

  } else {
    HTTPLOG_DEBUG("buffer full, skipping message\n");
    if(httpConn) {
      HTTPLOG_DEBUG("buffer full, httpConn->tcp state flags:%d timer:%d\n",httpConn->tcpstateflags, httpConn->timer);
    }
  }
  return 0;
}

uint8_t
httplog_P(const char *message, ...)
{
  if (httplog_tmp_buf == 0)
  {
    va_list va;
    va_start(va, message);
    int len=vsnprintf_P(NULL, 0, message, va);
    va_end(va);
    if(len >= HTTPLOG_BUFFER_LEN) {
      HTTPLOG_DEBUG("error: message too long %d\n", len);
      return 0;
    }
    uint8_t rc=httplog_buffer_empty(len+1);
    if(!rc) {
      HTTPLOG_DEBUG("error: allocating buffer\n");
   } else if(rc) {
      va_start(va, message);
      vsnprintf_P(httplog_tmp_buf, len+1, message, va);
      va_end(va);
      httplog_tmp_buf[len] = 0;
      HTTPLOG_DEBUG("httplog send message [%s] bufferlen=%d\n", httplog_tmp_buf, len);
      httplog_resolve_address();
      return 1;
    }

  } else {
    HTTPLOG_DEBUG("buffer full, skipping message\n");
    if(httpConn) {
      HTTPLOG_DEBUG("buffer full, httpConn->tcp state flags:%d timer:%d\n",httpConn->tcpstateflags, httpConn->timer);
    }
  }
  return 0;
}
