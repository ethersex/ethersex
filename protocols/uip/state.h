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
#include "services/pam/pam_ldap_state.h"



#endif /* CONNECTION_STATE_H */
