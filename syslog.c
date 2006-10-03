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

#include <stdint.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "uip/uip.h"
#include "syslog.h"

#ifdef DEBUG_SYSLOG
#include "uart.h"
#endif

const char syslog_message_boot[] PROGMEM = "etherrape startup";
const char syslog_sensor_message_open[] PROGMEM = "sensor %d open";
const char syslog_sensor_message_close[] PROGMEM = "sensor %d close";

void syslog_boot(void)
/* {{{ */ {

    uip_ipaddr_t syslog_server;
    uip_ipaddr(syslog_server, 172, 23, 23, 1);

    struct uip_udp_conn *c = uip_udp_new(&syslog_server, HTONS(SYSLOG_UDP_PORT));

    if (c != NULL) {
        uip_udp_bind(c, HTONS(SYSLOG_UDP_PORT));
        c->appstate.syslog.transmit_state = 0;
        c->appstate.syslog.state = SYSLOG_STATE_BOOT_MESSAGE;

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: udp connection prepared\r\n");
#endif
    }

} /* }}} */

void syslog_sensor(uint8_t num, uint8_t state)
/* {{{ */ {

    uip_ipaddr_t syslog_server;
    uip_ipaddr(syslog_server, 172, 23, 23, 1);

    struct uip_udp_conn *c = uip_udp_new(&syslog_server, HTONS(SYSLOG_UDP_PORT));

    if (c != NULL) {
        uip_udp_bind(c, HTONS(SYSLOG_UDP_PORT));
        c->appstate.syslog.transmit_state = 0;
        c->appstate.syslog.state = SYSLOG_STATE_SENSOR_MESSAGE;
        c->appstate.syslog.sensor = num;
        c->appstate.syslog.sensor_state = state;

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: udp connection prepared\r\n");
#endif
    }

} /* }}} */

void syslog_send_log(void)
/* {{{ */ {

    if (uip_udp_conn->appstate.syslog.transmit_state != 0) {
        uip_udp_remove(uip_udp_conn);
        return;
    }

    if (uip_udp_conn->appstate.syslog.state == SYSLOG_STATE_BOOT_MESSAGE) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: sending log\r\n");
#endif

        uint16_t length = sprintf_P(uip_appdata, syslog_message_boot);

        /* send packet */
        uip_udp_send(length);

    } else if (uip_udp_conn->appstate.syslog.state == SYSLOG_STATE_SENSOR_MESSAGE) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: sending log\r\n");
#endif

        uint16_t length;

        if (uip_udp_conn->appstate.syslog.sensor_state == 0)
            length = sprintf_P(uip_appdata, syslog_sensor_message_close,
                    uip_udp_conn->appstate.syslog.sensor);
        else
            length = sprintf_P(uip_appdata, syslog_sensor_message_open,
                    uip_udp_conn->appstate.syslog.sensor);

        /* send packet */
        uip_udp_send(length);

    }

} /* }}} */

void syslog_handle_conn(void)
/* {{{ */ {

    if (uip_poll()) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: transmit state is 0x");
        uart_puthexbyte(uip_udp_conn->appstate.syslog.transmit_state);
        uart_puts_P(", state is 0x");
        uart_puthexbyte(uip_udp_conn->appstate.syslog.state);
        uart_eol();
#endif

        syslog_send_log();
    }

} /* }}} */
