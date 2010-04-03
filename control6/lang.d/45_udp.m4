dnl
dnl  Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
dnl  Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 3 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl  For more information on the GPL, please go to:
dnl  http://www.gnu.org/copyleft/gpl.html
dnl

divert(0)

#define UDP_STATE (&uip_udp_conn->appstate.control6_udp)
#define UDP_BUF ((struct uip_udpip_hdr *) (uip_appdata - UIP_IPUDPH_LEN))

divert(-1)

dnl ==========================================================================
dnl UDP_EXPECT(text)
dnl ==========================================================================
define(`UDP_EXPECT', `
  PT_WAIT_UNTIL(pt, uip_len && uip_newdata () && strstr_P(uip_appdata, PSTR($1)) != NULL);
')


dnl ==========================================================================
dnl UDP_DIRECT_SEND(ip, lport, rport, text, ...)
dnl ==========================================================================
define(`UDP_DIRECT_SEND', `do { dnl
  ifelse(`$#', 4, dnl
	`strcpy_P(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], PSTR($4));
	 uip_slen = strlen ( &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN]);',
	dnl
	dnl multiple args, use printf syntax
	`uip_slen = snprintf_P ( &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], UIP_BUFSIZE - UIP_IPUDPH_LEN, PSTR($4), shift(shift(shift(shift($@)))));')

  { uip_udp_conn_t conn;
    IPADDR($1);
    uip_ipaddr_copy(conn.ripaddr, &ip);
    conn.lport = HTONS($2);
    conn.rport = HTONS($3);
    uip_udp_conn = &conn;
    uip_process(UIP_UDP_SEND_CONN);
    router_output();

    uip_slen = 0;
  }
  PT_YIELD(pt);
} while(0);
')


dnl ==========================================================================
dnl UDP_SEND(text, ...)
dnl ==========================================================================
define(`UDP_SEND', `do {dnl
  ifelse(`$#', 1, dnl
	`strcpy_P( &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], PSTR($1));
	 uip_udp_send (strlen ( &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN]));',
	dnl
	dnl multiple args, use printf syntax
	`uip_udp_send (snprintf_P ( &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], UIP_BUFSIZE - UIP_IPUDPH_LEN, PSTR($1), shift($@)));')

	{
         uip_udp_conn_t echo_conn;
         uip_ipaddr_copy(echo_conn.ripaddr, UDP_BUF->srcipaddr);
         echo_conn.rport = UDP_BUF->srcport;
         echo_conn.lport = uip_udp_conn->lport;
 
         uip_udp_conn = &echo_conn;
         uip_process(UIP_UDP_SEND_CONN);
         router_output();
         debug_printf("%s\n", uip_sappdata);
         uip_slen = 0;
	}

  PT_YIELD(pt);
} while(0)
')

dnl ==========================================================================
dnl UDP_HANDLER(name)
dnl ==========================================================================
define(`UDP_HANDLER', `dnl
divert(action_divert)dnl
static void
c6_udp_handler_$1 (void)
{

  PT_THREAD(inline_thread(struct pt *pt)) {
    PT_BEGIN(pt);
')

dnl ==========================================================================
dnl UDP_HANDLER_END()
dnl ==========================================================================
define(`UDP_HANDLER_END', `
    PT_END(pt);
  }

  if (uip_newdata ())
    ((char *) uip_appdata)[uip_len] = 0;

  inline_thread(&UDP_STATE->pt);
}
divert(normal_divert)')



dnl ==========================================================================
dnl UDP_LISTEN(port, handler)
dnl ==========================================================================
define(`UDP_LISTEN', `do {
     uip_ipaddr_t ip;
     uip_ipaddr_copy(&ip, all_ones_addr);

     uip_udp_conn_t *conn = uip_udp_new(&ip, 0, c6_udp_handler_$2);
     if(! conn) { 
	 debug_printf("control6: udp failed\n");
     }
     uip_udp_bind (conn, HTONS($1));
} while(0)
')

