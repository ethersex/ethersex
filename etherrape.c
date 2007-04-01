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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "uip/psock.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"

#include "config.h"
#include "global.h"
#include "debug.h"
#include "spi.h"
#include "network.h"

#include "net/handler.h"

#include "bit-macros.h"

/* global configuration */
global_config_t cfg;

/* prototypes */
void (*jump_to_bootloader)(void) = (void *)BOOTLOADER_SECTION;

/* macros */
#ifdef USE_WATCHDOG
#   define wdt_kick() wdt_reset()
#else
#   define wdt_kick()
#endif

int main(void)
/* {{{ */ {

    debug_init();
    debug_print("debugging enabled\n");

    /* enable interrupts */
    sei();

#   ifdef USE_WATCHDOG
    debug_print("enabling watchdog\n");

#   ifdef DEBUG

    /* for debugging, test reset cause and jump to bootloader */
    if (MCUSR & _BV(WDRF)) {
        debug_print("BUG: got reset by the watchdog!!\n");

        /* clear flags */
        MCUSR &= ~_BV(WDRF);

        debug_print("jumping to bootloader...\n");
        jump_to_bootloader();

    }
#   endif

    /* set watchdog to 2 seconds */
    wdt_enable(WDTO_2S);
    wdt_kick();
#   else
    debug_print("disabling watchdog\n");
    wdt_disable();
#   endif

    /* send boot message */
    debug_print("booting etherrape firmware " VERSION_STRING "...\n");

    spi_init();
    network_init();

    debug_printf("enc28j60 revision 0x%x\n", read_control_register(REG_EREVID));
    debug_printf("ip: %d.%d.%d.%d\n", LO8(uip_hostaddr[0]), HI8(uip_hostaddr[0]), LO8(uip_hostaddr[1]), HI8(uip_hostaddr[1]));

    while(1) /* main loop {{{ */ {

        wdt_kick();

        /* check for network controller interrupts */
        enc28j60_process_interrupts();

        /* check for timer interrupt */
        if (_TIFR_TIMER1 & _BV(OCF1A)) {

            static uint8_t c;

            c++;

#           if UIP_CONNS <= 255
            uint8_t i;
#           else
            uint16_t i;
#           endif

            /* periodically check connections */
            for (i = 0; i < UIP_CONNS; i++) {
                uip_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0) {
                    uip_arp_out();
                    transmit_packet();
                }
            }

#           if UIP_UDP == 1
            for (i = 0; i < UIP_UDP_CONNS; i++) {
                uip_udp_periodic(i);

                /* if this generated a packet, send it now */
                if (uip_len > 0) {
                    // XXX FIXME if (uip_arp_out() == 0)
                    // XXX FIXME     uip_udp_conn->appstate.sntp.transmit_state = 1;

                    transmit_packet();
                }
            }
#           endif

            // FIXME
            //if (c % 5 == 0) /* every second */
            //    clock_periodic();

            if (c == 50) { /* every 10 secs */
                uip_arp_timer();
                c = 0;
            }

            /* clear flag */
            _TIFR_TIMER1 = _BV(OCF1A);

        }

        wdt_kick();

        /* check for serial data */
        // if (_UCSRA_UART0 & _BV(_RXC_UART0)) {
        // 
        //     check_serial_input(_UDR_UART0);
        // 
        // }

        wdt_kick();

#       ifdef DEBUG
        /* if we changed to spi slave mode */
        if (!(_SPCR0 & _BV(_MSTR0))) {
            debug_print("switched to spi slave mode? no pullup at SS?\n");

            _SPCR0 |= _BV(_MSTR0);

        }
#       endif

        wdt_kick();

#       ifdef USE_74HC165
        /* check for extended io, if all sensors are closed */
        uint8_t status = hc165_read_byte();
        static uint8_t old_status;

        if (status == 0)
            PORTA &= ~_BV(PA4); /* green */
        else
            PORTA |= _BV(PA4);  /* red */

        for (uint8_t i = 0; i < 8; i++) {

            if (  (status & _BV(i)) != (old_status & _BV(i))  )
                syslog_sensor(i, status & _BV(i));

        }

        old_status = status;
#       endif

    } /* }}} */

} /* }}} */
