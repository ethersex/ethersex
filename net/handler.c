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
#include "bootp_net.h"
#include "tftp_net.h"

void network_init_apps(void)
/* {{{ */ {

#   ifdef ECMD_SUPPORT
    ecmd_net_init();
#   endif

#   ifdef TETRIRAPE_SUPPORT
    tetrirape_net_init();
#   endif

#   ifdef BOOTP_SUPPORT
    bootp_net_init();
#   endif

#   ifdef TFTP_SUPPORT
    tftp_net_init();
#   endif

    /* initialize your applications here */

} /* }}} */


#ifdef TCP_SUPPORT 
void network_handle_tcp(void)
/* {{{ */ {

#ifdef DEBUG_NET
    uart_puts_P("net_tcp: local port is 0x");
    uart_puthexbyte(HI8(uip_conn->lport));
    uart_puthexbyte(LO8(uip_conn->lport));
    uart_eol();
#endif

#   ifdef RC4_SUPPORT
    if(uip_connected()) {
        const char key[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
        /* new connection, initialize rc4 stream generators */
        rc4_init(&uip_conn->rc4_inbound, key, 16);
        rc4_init(&uip_conn->rc4_outbound, key, 16);
    }

    if(uip_rexmit()) {
        /* our outbound rc4 stream generator is out of sync, 
         * we cannot retransmit.  reset connection.  FIXME */
        uip_abort();
        return;
    }

    if(uip_newdata()) {
        /* new data for application, decrypt uip_len bytes from 
           uip_appdata on. */
        uint16_t i;
        for(i = 0; i < uip_len; i ++)
            ((char *) uip_appdata)[i] = 
              rc4_crypt_char(&uip_conn->rc4_inbound, ((char *) uip_appdata)[i]);
    }
#   endif /* RC4_SUPPORT */


    /* 
     * demultiplex packet
     */
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

#    ifdef RC4_SUPPORT
        /* new data from application, 
           encrypt uip_slen bytes from uip_sappdata on. */
     
     uint16_t i;
     for(i = 0; i < uip_slen; i ++)
         ((char *) uip_sappdata)[i] = 
           rc4_crypt_char(&uip_conn->rc4_outbound, 
                          ((char *) uip_sappdata)[i]);
#    endif /* RC4_SUPPORT */

} /* }}} */
#endif /* TCP_SUPPORT */


#ifdef UDP_SUPPORT
void network_handle_udp(void)
/* {{{ */ {

#   ifdef BOOTP_SUPPORT
    if (uip_udp_conn->lport == HTONS(BOOTPC_PORT))
	    bootp_net_main();
#   endif

#   ifdef TFTP_SUPPORT
    if (uip_udp_conn->lport == HTONS(TFTP_PORT)
        || uip_udp_conn->lport == HTONS(TFTP_ALT_PORT))
        tftp_net_main();
#   endif

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
#endif /* UDP_SUPPORT */

