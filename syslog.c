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
#include "eeprom.h"

#ifdef DEBUG_SYSLOG
#include "uart.h"
#endif

const char syslog_sensor_message_open[] PROGMEM = "sensor %d open";
const char syslog_sensor_message_close[] PROGMEM = "sensor %d close";

/* global variables */
uip_ipaddr_t syslog_server;

/* local prototypes */
struct uip_udp_conn *prepare_connection(void);
void syslog_send(void);


struct uip_udp_conn *prepare_connection(void)
/* {{{ */ {

    struct uip_udp_conn *c = uip_udp_new(&syslog_server, HTONS(SYSLOG_UDP_PORT));

    if (c != NULL) {
        uip_udp_bind(c, HTONS(SYSLOG_UDP_PORT));
        c->appstate.syslog.transmit_state = 0;

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: udp connection prepared\r\n");
#endif

        return c;
    } else
        return NULL;

} /* }}} */

void syslog_message(PGM_P str)
/* {{{ */ {

    struct uip_udp_conn *c = prepare_connection();

    if (c != NULL) {

        c->appstate.syslog.state = SYSLOG_STATE_MESSAGE;
        c->appstate.syslog.str = str;

    }

} /* }}} */

void syslog_sensor(uint8_t num, uint8_t state)
/* {{{ */ {

    struct uip_udp_conn *c = prepare_connection();

    if (c != NULL) {

        c->appstate.syslog.state = SYSLOG_STATE_SENSOR_MESSAGE;
        c->appstate.syslog.sensor = num;
        c->appstate.syslog.sensor_state = state;

    }

} /* }}} */

void syslog_send(void)
/* {{{ */ {

    if (uip_udp_conn->appstate.syslog.transmit_state != 0) {
        uip_udp_remove(uip_udp_conn);
        return;
    }

    if (uip_udp_conn->appstate.syslog.state == SYSLOG_STATE_MESSAGE) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: sending normal message\r\n");
#endif

        strcpy_P(uip_appdata, uip_udp_conn->appstate.syslog.str);
        uint16_t length = strlen_P(uip_udp_conn->appstate.syslog.str);

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

        syslog_send();
    }

} /* }}} */
