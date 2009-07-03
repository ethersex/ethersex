/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef CONNECTION_STATE_H
#define CONNECTION_STATE_H

#include "protocols/uip/uip_openvpn.h"
#include "config.h"

#include "protocols/ecmd/via_tcp/ecmd_state.h"
#include "protocols/ecmd/sender/ecmd_sender_state.h"
#include "hardware/i2c/master/i2c_state.h"
#include "hardware/i2c/slave/i2c_slave_state.h"
#include "hardware/radio/rfm12/rfm12_raw_state.h"
#include "protocols/bootp/bootp_state.h"
#include "protocols/modbus/modbus_state.h"
#include "protocols/zbus/zbus_raw_state.h"
#include "protocols/mysql/mysql_state.h"
#include "services/dyndns/dyndns_state.h"
#include "services/tftp/tftp_state.h"
#include "services/httpd/httpd_state.h"
#include "services/jabber/jabber_state.h"
#include "protocols/smtp/sendmail.h"
#include "protocols/irc/irc_state.h"
#include "core/tty/tty-vt100-telnet.h"
#include "core/pt/pt.h"

/* uip appstate for tcp */
typedef union uip_tcp_connection_state {

#   ifdef ECMD_TCP_SUPPORT
    struct ecmd_connection_state_t ecmd;
#   endif

#   ifdef ECMD_SENDER_SUPPORT
    struct ecmd_sender_connection_state_t ecmd_sender;
#   endif

#   if defined(DYNDNS_SUPPORT) && defined(TCP_SUPPORT) \
       && !defined(TEENSY_SUPPORT)
    struct dyndns_connection_state_t dyndns;
#   endif

#   ifdef MODBUS_SUPPORT
    struct modbus_connection_state_t modbus;
#   endif

#   ifdef HTTPD_SUPPORT
    struct httpd_connection_state_t httpd;
#   endif

#   ifdef JABBER_SUPPORT
    struct jabber_connection_state_t jabber;
#   endif

#   ifdef MYSQL_SUPPORT
    struct mysql_connection_state_t mysql;
#   endif

#   ifdef SENDMAIL_SUPPORT
    struct sendmail_connection_state_t sendmail;
#   endif

#   ifdef IRC_SUPPORT
    struct irc_connection_state_t irc;
#   endif

#   ifdef TTY_LL_VT100_TELNET
    struct tty_vt100_state_t tty_vt100;
#   endif

#   ifdef CONTROL6_SUPPORT
    struct {
	struct pt pt;
	lc_t rexmit_lc;
    } control6_tcp;
#   endif
} uip_tcp_appstate_t;


/* uip appstate for udp */
typedef union uip_udp_connection_state {

#   ifdef BOOTP_SUPPORT
    struct bootp_connection_state_t bootp;
#   endif

#   ifdef TFTP_SUPPORT
    struct tftp_connection_state_t tftp;
#   endif

#   ifdef I2C_UDP_SUPPORT
    struct i2c_connection_state_t i2c;
#   endif

#   ifdef I2C_SLAVE_SUPPORT
    struct i2c_slave_connection_state_t i2c_slave;
#   endif

#   ifdef OPENVPN_SUPPORT
    struct openvpn_connection_state_t openvpn;
#   endif

#   ifdef RFM12_RAW_SUPPORT
    struct rfm12_raw_connection_state_t rfm12_raw;
#   endif

#   ifdef ZBUS_RAW_SUPPORT
    struct zbus_raw_connection_state_t zbus_raw;
#   endif

} uip_udp_appstate_t;

#endif /* CONNECTION_STATE_H */
