/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

#include "protocols/uip/uip.h"
#include "core/bit-macros.h"
#include "core/eeprom.h"
#include "core/periodic.h"

#include "dhcp_state.h"
#include "dhcp.h"

#include "services/ntp/ntp.h"

#define STATE_INITIAL         0
#define STATE_DISCOVERING     1
#define STATE_REQUESTING      2
#define STATE_CONFIGURED      3

/* Lease renewal timing per RFC 2131 */
#define LEASE_RENEW_T1(lease) ((lease) / 2)      /* 50% - start renewing */
#define LEASE_REBIND_T2(lease) ((lease) * 7 / 8) /* 87.5% - start rebinding */

/* Maximum values for overflow protection */
#define MAX_TICKS32 (~((uint32_t)0))

struct dhcp_msg {
  uint8_t op, htype, hlen, hops;
  uint8_t xid[4];
  uint16_t secs, flags;
  uint8_t ciaddr[4];
  uint8_t yiaddr[4];
  uint8_t siaddr[4];
  uint8_t giaddr[4];
  uint8_t chaddr[16];
#ifndef UIP_CONF_DHCP_LIGHT
  uint8_t sname[64];
  uint8_t file[128];
#endif
  uint8_t options[312];
};

#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_HOSTNAME     12
#define DHCP_OPTION_NTP_SERVER   42
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_CLIENT_ID    61
#define DHCP_OPTION_END         255

static const uint8_t xid[4] = {0xad, 0xde, 0x12, 0x23};
static const uint8_t magic_cookie[4] = {99, 130, 83, 99};

static uint32_t tick_sec;

/*---------------------------------------------------------------------------*/
static uint8_t *
add_msg_type(uint8_t *optptr, uint8_t type)
{
  *optptr++ = DHCP_OPTION_MSG_TYPE;
  *optptr++ = 1;
  *optptr++ = type;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_server_id(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_SERVER_ID;
  *optptr++ = 4;
  memcpy(optptr, uip_udp_conn->appstate.dhcp.serverid, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_req_ipaddr(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_IPADDR;
  *optptr++ = 4;
  memcpy(optptr, uip_udp_conn->appstate.dhcp.ipaddr, 4);
  return optptr + 4;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_req_options(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_REQ_LIST;
  *optptr++ = 4;
  *optptr++ = DHCP_OPTION_SUBNET_MASK;
  *optptr++ = DHCP_OPTION_ROUTER;
  *optptr++ = DHCP_OPTION_DNS_SERVER;
  *optptr++ = DHCP_OPTION_NTP_SERVER;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_hostname(uint8_t *optptr)
{
  int len = sizeof(CONF_HOSTNAME);
  *optptr++ = DHCP_OPTION_HOSTNAME;
  *optptr++ = len;
  memcpy_P(optptr, PSTR(CONF_HOSTNAME), len);
  return optptr + len;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_client_id(uint8_t *optptr)
{
  int len = 1+sizeof(struct uip_eth_addr); /* hardware type + MAC-Adress */
  *optptr++ = DHCP_OPTION_CLIENT_ID;
  *optptr++ = len;
  /* hardware type: ether */
  optptr[0] = 1;
  /* mac address */
  memcpy(&optptr[1], uip_ethaddr.addr, sizeof(struct uip_eth_addr));
  return optptr + len;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
add_end(uint8_t *optptr)
{
  *optptr++ = DHCP_OPTION_END;
  return optptr;
}
/*---------------------------------------------------------------------------*/
static void create_msg(register struct dhcp_msg *m)
{
  m->op = DHCP_REQUEST;
  m->htype = DHCP_HTYPE_ETHERNET;
  m->hlen = sizeof(struct uip_eth_addr);
  m->hops = 0;
  memcpy(m->xid, xid, sizeof(m->xid));
  m->secs = 0;
  m->flags = HTONS(BOOTP_BROADCAST); /*  Broadcast bit. */
  /*  uip_ipaddr_copy(m->ciaddr, uip_hostaddr);*/
  memcpy(m->ciaddr, uip_hostaddr, sizeof(m->ciaddr));
  memset(m->yiaddr, 0, sizeof(m->yiaddr));
  memset(m->siaddr, 0, sizeof(m->siaddr));
  memset(m->giaddr, 0, sizeof(m->giaddr));
  memcpy(m->chaddr, uip_ethaddr.addr, sizeof(struct uip_eth_addr));
  memset(&m->chaddr[sizeof(struct uip_eth_addr)], 0, sizeof(m->chaddr) - sizeof(struct uip_eth_addr));
#ifndef UIP_CONF_DHCP_LIGHT
  memset(m->sname, 0, sizeof(m->sname));
  memset(m->file, 0, sizeof(m->file));
#endif

  memcpy(m->options, magic_cookie, sizeof(magic_cookie));
}

/*---------------------------------------------------------------------------*/
static void send_discover(void) {
  uint8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  create_msg(m);
  
  end = add_msg_type(&m->options[4], DHCPDISCOVER);
  end = add_req_options(end);
  end = add_end(end);
  
  uip_send(uip_appdata, end - (uint8_t *)uip_appdata);
}

/*---------------------------------------------------------------------------*/
static void
send_request(void)
{
  uint8_t *end;
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;

  create_msg(m);
  
  end = add_msg_type(&m->options[4], DHCPREQUEST);
  end = add_server_id(end);
  end = add_req_ipaddr(end);
  end = add_hostname(end);
  end = add_client_id(end);
  end = add_end(end);
  
  uip_send(uip_appdata, end - (uint8_t *)uip_appdata);
}
/*---------------------------------------------------------------------------*/
static uint8_t
parse_options(uint8_t *optptr, int len)
{
  uint8_t *end = optptr + len;
  uint8_t type = 0;

  while(optptr < end) {
    switch(*optptr) {
    case DHCP_OPTION_SUBNET_MASK:
      memcpy(uip_udp_conn->appstate.dhcp.netmask, optptr + 2, 4);
      break;
    case DHCP_OPTION_ROUTER:
      memcpy(uip_udp_conn->appstate.dhcp.default_router, optptr + 2, 4);
      break;
    case DHCP_OPTION_DNS_SERVER:
      memcpy(uip_udp_conn->appstate.dhcp.dnsaddr, optptr + 2, 4);
      break;
    case DHCP_OPTION_MSG_TYPE:
      type = *(optptr + 2);
      break;
    case DHCP_OPTION_SERVER_ID:
      memcpy(uip_udp_conn->appstate.dhcp.serverid, optptr + 2, 4);
      break;
    case DHCP_OPTION_NTP_SERVER:
      memcpy(uip_udp_conn->appstate.dhcp.ntpaddr, optptr + 2, 4);
      break;
    case DHCP_OPTION_LEASE_TIME:
      memcpy(uip_udp_conn->appstate.dhcp.lease_time, optptr + 2, 4);
      break;
    case DHCP_OPTION_END:
      return type;
    }

    optptr += optptr[1] + 2;
  }
  return type;
}
/*---------------------------------------------------------------------------*/
static uint8_t
parse_msg(void)
{
  struct dhcp_msg *m = (struct dhcp_msg *)uip_appdata;
  
  if(m->op == DHCP_REPLY &&
     memcmp(m->xid, xid, sizeof(xid)) == 0 &&
     memcmp(m->chaddr, uip_ethaddr.addr, sizeof(struct uip_eth_addr)) == 0) {
    memcpy(uip_udp_conn->appstate.dhcp.ipaddr, m->yiaddr, 4);
    return parse_options(&m->options[4], uip_datalen());
  }
  return 0;
}

void dhcp_periodic(void) {
  /* This is called 50 times per second (HZ=50) */
  /* We want tick_sec to count actual seconds, so only increment every HZ calls */
  static uint8_t counter = 0;
  counter++;
  if (counter >= HZ) {
    tick_sec++;
    counter = 0;
    /* Prevent overflow */
    if(tick_sec > MAX_TICKS32 / 2) {
      tick_sec = MAX_TICKS32 / 2;
    }
  }
}

void dhcp_set_static(void) {
  uip_ipaddr_t ip;

  /* Configure the IP address. */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip(ip, &ip);
#else
  set_CONF_ETHERSEX_IP(&ip);
#endif
  uip_sethostaddr(&ip);
  
  
  /* Configure the netmask (IPv4). */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip(netmask, &ip);
#else
  set_CONF_ETHERSEX_IP4_NETMASK(&ip);
#endif
  uip_setnetmask(&ip);
  
  /* Configure the default gateway  */
#ifdef EEPROM_SUPPORT
  /* Please Note: ip and &ip are NOT the same (cpp hell) */
  eeprom_restore_ip(gateway, &ip);
#else
  set_CONF_ETHERSEX_GATEWAY(&ip);
#endif
  uip_setdraddr(&ip);
  
  /* Remove the bootp connection */
  uip_udp_remove(uip_udp_conn);

}

void dhcp_net_init(void) {

#ifdef S0BRIDGE_SUPPORT
  uint8_t ips = 0;
  eeprom_restore_int( ip_static, &ips );

  if (ips) {
    dhcp_set_static();
    return;
  }
#endif 

  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);
  
  uip_udp_conn_t *dhcp_conn = uip_udp_new(&ip, HTONS(DHCPC_SERVER_PORT), dhcp_net_main);
  
  if(! dhcp_conn)
    return; /* dammit. */
  
  uip_udp_bind(dhcp_conn, HTONS(DHCPC_CLIENT_PORT));
  
  dhcp_conn->appstate.dhcp.retry_counter = 0;
  dhcp_conn->appstate.dhcp.retry_timer = 5;
  dhcp_conn->appstate.dhcp.state = STATE_INITIAL;
  dhcp_conn->appstate.dhcp.lease_time_seconds = 0;
  dhcp_conn->appstate.dhcp.ticks = 0;
  tick_sec = 0;
}


void dhcp_net_main(void) {
  /* Contiki-inspired DHCP state machine with lease renewal per RFC 2131 */

  if(uip_newdata()) {
    uint8_t msg_type = parse_msg();

    switch (uip_udp_conn->appstate.dhcp.state) {
    case STATE_DISCOVERING:
      if (msg_type == DHCPOFFER) {
        send_request();
        uip_flags &= ~UIP_NEWDATA;
        uip_udp_conn->appstate.dhcp.state = STATE_REQUESTING;
        uip_udp_conn->appstate.dhcp.retry_timer = 2;
        uip_udp_conn->appstate.dhcp.retry_counter = 1;
        tick_sec = 0;
      }
      break;

    case STATE_REQUESTING:
    case STATE_CONFIGURED:
      if (msg_type == DHCPACK) {
        /* Configure network with received parameters */
        uip_sethostaddr(uip_udp_conn->appstate.dhcp.ipaddr);
        uip_setdraddr(uip_udp_conn->appstate.dhcp.default_router);
        uip_setnetmask(uip_udp_conn->appstate.dhcp.netmask);

#ifdef DNS_SUPPORT
        resolv_conf(uip_udp_conn->appstate.dhcp.dnsaddr);
#endif

#ifdef NTP_SUPPORT
        ntp_conf(uip_udp_conn->appstate.dhcp.ntpaddr);
#endif

        uip_udp_conn->appstate.dhcp.state = STATE_CONFIGURED;
        uip_flags &= ~UIP_NEWDATA;

        /* Calculate lease time in seconds from network byte order */
        uip_udp_conn->appstate.dhcp.lease_time_seconds = 
          (uint32_t)ntohs(uip_udp_conn->appstate.dhcp.lease_time[0]) * 65536UL +
          ntohs(uip_udp_conn->appstate.dhcp.lease_time[1]);

        /* Start with elapsed = 0, will count up to T1, T2, then lease expiration */
        uip_udp_conn->appstate.dhcp.ticks = 0;
        tick_sec = 0;

        /* Do NOT remove the UDP connection - needed for lease renewal */
      }
      break;
    }
  } else {
    /* No data - handle timeouts and state transitions */

    switch (uip_udp_conn->appstate.dhcp.state) {
    case STATE_INITIAL:
    case STATE_DISCOVERING:
      if (tick_sec > uip_udp_conn->appstate.dhcp.retry_timer) {
        send_discover();
        uip_flags &= ~UIP_NEWDATA;
        uip_udp_conn->appstate.dhcp.state = STATE_DISCOVERING;
        if (uip_udp_conn->appstate.dhcp.retry_counter++ > 10) {
          return dhcp_set_static();
        }
        uip_udp_conn->appstate.dhcp.retry_timer = 
          2 * uip_udp_conn->appstate.dhcp.retry_counter;
        tick_sec = 0;
      }
      break;

    case STATE_REQUESTING:
      if (tick_sec > uip_udp_conn->appstate.dhcp.retry_timer) {
        send_request();
        uip_flags &= ~UIP_NEWDATA;
        if (uip_udp_conn->appstate.dhcp.retry_counter++ > 10) {
          return dhcp_set_static();
        }
        uip_udp_conn->appstate.dhcp.retry_timer = 2;
        tick_sec = 0;
      }
      break;

    case STATE_CONFIGURED:
      /* Lease renewal logic per RFC 2131 */
      if (uip_udp_conn->appstate.dhcp.lease_time_seconds > 0) {
        uint32_t lease = uip_udp_conn->appstate.dhcp.lease_time_seconds;
        uint32_t t1 = LEASE_RENEW_T1(lease);
        uint32_t t2 = LEASE_REBIND_T2(lease);
        uint32_t elapsed = uip_udp_conn->appstate.dhcp.ticks + tick_sec;
        
        /* Check if we've reached T1 (50% of lease) - renewing phase */
        if (elapsed >= t1 && uip_udp_conn->appstate.dhcp.ticks < t1) {
          /* First time reaching T1 - send renewal request */
          send_request();
          uip_flags &= ~UIP_NEWDATA;
          uip_udp_conn->appstate.dhcp.ticks = t1; /* Mark that we've passed T1 */
          tick_sec = 0;
        }
        /* Check if we've reached T2 (87.5% of lease) - rebinding phase */
        else if (elapsed >= t2 && uip_udp_conn->appstate.dhcp.ticks < t2) {
          /* First time reaching T2 - send rebinding request */
          send_request();
          uip_flags &= ~UIP_NEWDATA;
          uip_udp_conn->appstate.dhcp.ticks = t2; /* Mark that we've passed T2 */
          tick_sec = 0;
        }
        /* Check if lease has expired */
        else if (elapsed >= lease) {
          /* Lease expired - restart DHCP from beginning */
          uip_udp_conn->appstate.dhcp.state = STATE_INITIAL;
          uip_udp_conn->appstate.dhcp.retry_counter = 0;
          uip_udp_conn->appstate.dhcp.retry_timer = 5;
          uip_udp_conn->appstate.dhcp.ticks = 0;
          tick_sec = 0;
        }
        
        /* If we haven't reached any threshold yet, just accumulate time */
        if (elapsed < lease) {
          uip_udp_conn->appstate.dhcp.ticks = elapsed;
          tick_sec = 0;
        }
      }
      break;
    }
  }
}



/*
  -- Ethersex META --
  header(protocols/dhcp/dhcp.h)
  net_init(dhcp_net_init)
  timer(50, dhcp_periodic())

  state_header(protocols/dhcp/dhcp_state.h)
  state_udp(struct dhcp_connection_state_t dhcp)
*/
