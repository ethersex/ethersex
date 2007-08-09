/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#include "handler.h"
#include "../uip/uip.h"

#include "ecmd_net.h"
#include "tetrirape_net.h"

void network_init_apps(void)
/* {{{ */ {

#   ifdef ECMD_SUPPORT
    ecmd_net_init();
#   endif

#   ifdef TETRIRAPE_SUPPORT
    tetrirape_net_init();
#   endif

    /* initialize your applications here */

} /* }}} */

void network_handle_tcp(void)
/* {{{ */ {

#ifdef DEBUG_NET
    uart_puts_P("net_tcp: local port is 0x");
    uart_puthexbyte(HI8(uip_conn->lport));
    uart_puthexbyte(LO8(uip_conn->lport));
    uart_eol();
#endif

#   ifdef ECMD_SUPPORT
    if (uip_conn->lport == HTONS(ECMD_NET_PORT))
        ecmd_net_main();
#   endif

#   ifdef TETRIRAPE_SUPPORT
    if (uip_conn->lport == HTONS(TETRIRAPE_PORT))
        tetrirape_net_main();
#   endif

    /* put tcp application calls here, example:
     *
     * if (uip_conn->lport == HTONS(ETHCMD_PORT))
     *     ethcmd_main();
     *
     * if (uip_conn->lport == HTONS(HTTPD_PORT) ||
     *     uip_conn->lport == HTONS(HTTPD_ALTERNATE_PORT))
     *         httpd_main();
     */

} /* }}} */

void network_handle_udp(void)
/* {{{ */ {

    /* put udp application calls here, example:
     *
     * if (uip_udp_conn->lport == HTONS(SNTP_UDP_PORT))
     *     sntp_handle_conn();
     * 
     * if (uip_udp_conn->lport == HTONS(SYSLOG_UDP_PORT))
     *     syslog_handle_conn();
     * 
     * if (uip_udp_conn->lport == HTONS(FC_UDP_PORT))
     *     fc_handle_conn();
     */

} /* }}} */
