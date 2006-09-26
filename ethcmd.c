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

#include "ethcmd.h"
#include "uip/uip.h"
#include "uart.h"

void ethcmd_init(void)
/* {{{ */ {

    uip_listen(HTONS(ETHCMD_PORT));

} /* }}} */

void ethcmd_main(void)
/* {{{ */ {

    if (uip_poll())
        return;

    uart_puts_P("cmd: main(), uip_flags 0x");
    uart_puthexbyte(uip_flags);
    uart_eol();

    if (uip_aborted())
        uart_puts_P("cmd: connection aborted\r\n");

    if (uip_timedout())
        uart_puts_P("cmd: connection aborted\r\n");

    if (uip_closed())
        uart_puts_P("cmd: connection closed\r\n");

    if (uip_connected())
        uart_puts_P("cmd: new connection\r\n");

} /* }}} */
