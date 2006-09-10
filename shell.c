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

#include <string.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "shell.h"
#include "uart.h"

void shell_init(void)
/* {{{ */ {

    uip_listen(HTONS(23));
    uip_listen(HTONS(60023));

#if UIP_UDP == 1
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);
    struct uip_udp_conn *c = uip_udp_new(&ip, 0);

    if (c != NULL)
        uip_udp_bind(c, HTONS(7));
    else
        uart_puts_P("shell_udp: udp_new returned NULL\r\n");
#endif

} /* }}} */

void shell_main(void)
/* {{{ */ {

    if (uip_connected()) {
        uart_puts_P("shell: new connection: ");
        uart_puts_ip(&uip_conn->ripaddr);
        uart_putc(':');
        uart_putdecbyte((uint8_t)(uip_conn->rport >> 8));
        uart_putdecbyte((uint8_t)(uip_conn->rport));
        uart_eol();
        uip_conn->appstate.state = SHELL_WELCOME;
    }

#if 0
    if (uip_poll()) {
        uart_puts_P("shell: poll()\r\n");
    }
#endif

    if (uip_newdata()) {
        char *str = ((char *)uip_appdata);

        uart_puts_P("shell: new data arrived: ");

        for (uint16_t i = 0; i < uip_datalen(); i++)
            if (str[i] >= 32)
                uart_putc(str[i]);

        uart_eol();

        if (strncasecmp_P(str, PSTR("status"), 6) == 0)
            uip_conn->appstate.state = SHELL_STATUS;
        else if (strncasecmp_P(str, PSTR("help"), 4) == 0)
            uip_conn->appstate.state = SHELL_HELP;
        else if (strncasecmp_P(str, PSTR("uptime"), 4) == 0)
            uip_conn->appstate.state = SHELL_UPTIME;
        else if (strncasecmp_P(str, PSTR("exit"), 4) == 0
              || strncasecmp_P(str, PSTR("quit"), 4) == 0)
            uip_conn->appstate.state = SHELL_EXIT;
        else if (strncasecmp_P(str, PSTR("sensors"), 7) == 0)
            uip_conn->appstate.state = SHELL_SENSORS;
        else
            uip_send("unknown command, try HELP\r\n> ", 29);
    }

    if (uip_closed() || uip_aborted()) {
        uart_puts_P("shell: connection closed from: ");
        uart_puts_ip(&uip_conn->ripaddr);
        uart_putc(':');
        uart_putdecbyte((uint8_t)(uip_conn->rport >> 8));
        uart_putdecbyte((uint8_t)(uip_conn->rport));
        uart_eol();
    }

    if (uip_timedout())
        uart_puts_P("shell: connection timed out\r\n");

    if (uip_acked()) {
        if (uip_conn->appstate.state == SHELL_EXIT)
            uip_close();
        else
            uip_conn->appstate.state = SHELL_IDLE;
    }

    if (uip_rexmit() || uip_connected() || uip_newdata())
        shell_send_response();

} /* }}} */

void shell_send_response(void)
/* {{{ */ {

    if (uip_conn->appstate.state == SHELL_WELCOME)
        uip_send("welcome on the etherrape telnet server! (try HELP)\r\n> ", 54);
    else if (uip_conn->appstate.state == SHELL_HELP) {
        uip_send("commands: HELP, STATUS, UPTIME, SENSORS, EXIT\r\n> ", 49);
#if 0
    else if (uip_conn->appstate.state == SHELL_UPTIME) {

        uint32_t time = uptime;

        uint8_t days = time / 86400;
        time = time - days * 86400;
        uint8_t hours = time / 3600;
        time = time % 3600;
        uint8_t min = time / 60;
        uint8_t sec = time % 60;

        uint8_t len = sprintf_P(uip_appdata, PSTR("uptime: %d days, %d hours, %d mins, %d secs\r\n> "), days, hours, min, sec);

        uip_send(uip_appdata, len);

#endif
    } else if (uip_conn->appstate.state == SHELL_EXIT) {
        uip_send("bye...\r\n> ", 8);
    } else if (uip_conn->appstate.state == SHELL_STATUS)
        uip_send("everything normal!\r\n> ", 22);
    else if (uip_conn->appstate.state == SHELL_SENSORS) {

        char *s = uip_appdata;

        DDRA &= ~(_BV(PA6) | _BV(PA7));
        PORTA |= (_BV(PA6) | _BV(PA7));

        uint8_t len = sprintf_P(s, PSTR("sensor1: "));

        if (PINA & _BV(PA7))
            len = len + sprintf_P(&s[len], PSTR("open"));
        else
            len = len + sprintf_P(&s[len], PSTR("closed"));

        len = len + sprintf_P(&s[len], PSTR(", sensor2: "));

        if (PINA & _BV(PA6))
            len = len + sprintf_P(&s[len], PSTR("open"));
        else
            len = len + sprintf_P(&s[len], PSTR("closed"));

        len = len + sprintf_P(&s[len], PSTR("\r\n> "));

        uip_send(uip_appdata, len);
    }

} /* }}} */

void shell_handle_udp(void)
/* {{{ */ {

    if (uip_connected())
        uip_send("welcome on the etherrape telnet server! (try HELP)\r\n> ", 54);

    if (uip_newdata()) {
        uart_puts_P("shell: udp packet received from:");
        uart_puts_ip(&uip_conn->ripaddr);
        uart_putc(':');
        uart_putdecbyte((uint8_t)(uip_conn->rport >> 8));
        uart_putdecbyte((uint8_t)(uip_conn->rport));
        uart_eol();

        uip_udp_send(uip_len);
    }

} /* }}} */
