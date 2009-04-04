/* sts=4 sw=4
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (C) 2007,2008 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <stdlib.h>

#include "handler.h"
#include "../uip/uip.h"
#include "../crypto/cast5.h"

#include "ecmd_net.h"
#include "uecmd_net.h"
#include "../bootp/bootp_net.h"
#include "../tftp/tftp_net.h"
#include "ecmd_sender_net.h"
#include "../dns/dns_net.h"
#include "../syslog/syslog_net.h"
#include "../i2c_master/i2c_udp_net.h"
#include "../i2c_slave/i2c_slave_net.h"
#include "../ntp/ntp_net.h"
#include "../ntp/ntpd_net.h"
#include "udp_echo_net.h"
#include "../yport/yport_net.h"
#include "../mdns_sd/mdns_sd_net.h"
#include "../dyndns/dyndns.h"
#include "../modbus/modbus_net.h"
#include "../rfm12/rfm12_raw_net.h"
#include "../zbus/zbus_raw_net.h"
#include "../stella/stella_net.h"
#include "../snmp/snmp_net.h"
#include "../mcuf/mcuf_net.h"
#include "../httpd/httpd.h"
#include "../jabber/jabber.h"
#include "../mysql/mysql.h"
#include "../debug.h"

/* Define this, if you want every fifth packet to be discarded. */
#undef  NETWORK_DEBUG_DISCARD_SOME

void network_init_apps(void)
{
#   ifdef ECMD_SUPPORT
    ecmd_net_init();
#   endif

#   ifdef JABBER_SUPPORT
    jabber_init();
#   endif

#   ifdef HTTPD_SUPPORT
    httpd_init();
#   endif

#   ifdef MYSQL_SUPPORT
    mysql_init();
#   endif

#   ifdef UECMD_SUPPORT
    uecmd_net_init();
#   endif

#   ifdef TFTP_SUPPORT
    tftp_net_init();
#   endif

#   ifdef MCUF_SUPPORT
    mcuf_net_init();
#   endif

#   ifdef DNS_SUPPORT
    dns_net_init();
#   endif

#   ifdef MDNS_SD_SUPPORT
    mdns_sd_net_init();
#   endif

#   ifdef YPORT_SUPPORT
    yport_net_init();
#   endif

#   ifdef SYSLOG_SUPPORT
    syslog_net_init();
#   endif

#   ifdef I2C_UDP_SUPPORT
    i2c_udp_net_init();
#   endif

#   ifdef I2C_SLAVE_SUPPORT
    i2c_slave_net_init();
#   endif

#   ifdef RFM12_RAW_SUPPORT
    rfm12_raw_net_init();
#   endif

#   ifdef ZBUS_RAW_SUPPORT
    zbus_raw_net_init();
#   endif

#   ifdef STELLA_SUPPORT
    stella_net_init();
#   endif

#   ifdef UDP_ECHO_NET_SUPPORT
    udp_echo_net_init();
#   endif

#   ifdef BOOTP_SUPPORT
    bootp_net_init();
#   endif

#   ifdef NTP_SUPPORT
    ntp_net_init();
#   endif

#   ifdef NTPD_SUPPORT
    ntpd_net_init();
#   endif

#   ifdef MODBUS_SUPPORT
    modbus_net_init();
#   endif

#   ifdef SNMP_SUPPORT
    snmp_net_init();
#   endif

#   if defined(DYNDNS_SUPPORT) && !defined(BOOTP_SUPPORT) \
      && ((!defined(IPV6_SUPPORT)) || defined(IPV6_STATIC_SUPPORT))
    dyndns_update();
#   endif
}


#ifdef NETWORK_DEBUG_DISCARD_SOME
static void network_debug_discard_some(void)
{
  if (! uip_slen) return;

  static int i = 0;
  if (++ i < 5) return;
  i = 0;

  /* destroy the packet/ */
  uip_slen = 0;
}
#else
#define network_debug_discard_some(a) (void)(0)
#endif


#ifdef TCP_SUPPORT 
void network_handle_tcp(void)
{

#ifdef DEBUG_NET
    debug_printf ("net_tcp: local port is 0x%04x\n", uip_conn->lport);
#endif

    /* 
     * demultiplex packet
     */
    if (uip_conn->callback != NULL) 
        uip_conn->callback();

    network_debug_discard_some();

}
#endif /* TCP_SUPPORT */


#ifdef UDP_SUPPORT
void network_handle_udp(void)
{
    if (uip_udp_conn->callback)
        uip_udp_conn->callback();

    network_debug_discard_some();

}
#endif /* UDP_SUPPORT */

