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
struct global_syslog_t global_syslog;

/* local prototypes */
struct uip_udp_conn *prepare_connection(void);
void syslog_send(void);


struct uip_udp_conn *prepare_connection(void)
/* {{{ */ {

    /* if syslog is disabled, return null */
    if (global_syslog.server[0] == 0 &&
            global_syslog.server[1] == 0)
        return NULL;

    struct uip_udp_conn *c = uip_udp_new(&global_syslog.server, HTONS(SYSLOG_UDP_PORT));

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

    if (!global_syslog.enabled) {
#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: disabled, unable to send message: ");
        uart_putf((void *)str);
        uart_eol();
#endif
        return;
    }

    struct uip_udp_conn *c = prepare_connection();

    if (c != NULL) {

        c->appstate.syslog.state = SYSLOG_STATE_MESSAGE;
        c->appstate.syslog.retransmit_counter = 0;
        c->appstate.syslog.retransmit_timeout = 0;
        c->appstate.syslog.str = str;

    }

} /* }}} */

void syslog_sensor(uint8_t num, uint8_t state)
/* {{{ */ {

    if (!global_syslog.enabled) {
#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: disabled, unable to send sensor message...\r\n");
#endif
        return;
    }

    struct uip_udp_conn *c = prepare_connection();

    if (c != NULL) {

        c->appstate.syslog.state = SYSLOG_STATE_SENSOR_MESSAGE;
        c->appstate.syslog.sensor = num;
        c->appstate.syslog.sensor_state = state;

    }

} /* }}} */

void syslog_send(void)
/* {{{ */ {

    if (uip_udp_conn->appstate.syslog.transmit_state != 0 ||
            uip_udp_conn->appstate.syslog.retransmit_counter == SYSLOG_RETRANSMIT_COUNTER) {
        uip_udp_remove(uip_udp_conn);
        return;
    }

    uint16_t length = 0;

    if (uip_udp_conn->appstate.syslog.state == SYSLOG_STATE_MESSAGE) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: sending normal message, retransmit_counter 0x");
        uart_puthexbyte(uip_udp_conn->appstate.syslog.retransmit_counter);
        uart_puts_P(", retransmit_timeout: 0x");
        uart_puthexbyte(uip_udp_conn->appstate.syslog.retransmit_timeout);
        uart_eol();
#endif

        strcpy_P(uip_appdata, uip_udp_conn->appstate.syslog.str);
        length = strlen_P(uip_udp_conn->appstate.syslog.str);

    } else if (uip_udp_conn->appstate.syslog.state == SYSLOG_STATE_SENSOR_MESSAGE) {

#ifdef DEBUG_SYSLOG
        uart_puts_P("syslog: sending log\r\n");
#endif

        if (uip_udp_conn->appstate.syslog.sensor_state == 0)
            length = sprintf_P(uip_appdata, syslog_sensor_message_close,
                    uip_udp_conn->appstate.syslog.sensor);
        else
            length = sprintf_P(uip_appdata, syslog_sensor_message_open,
                    uip_udp_conn->appstate.syslog.sensor);
    }

    if (uip_udp_conn->appstate.syslog.retransmit_timeout == 0) {

        /* send packet */
        uip_udp_send(length);
        uip_udp_conn->appstate.syslog.retransmit_counter++;
        uip_udp_conn->appstate.syslog.retransmit_timeout = SYSLOG_RETRANSMIT_TIMEOUT;

    } else
        uip_udp_conn->appstate.syslog.retransmit_timeout--;

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
