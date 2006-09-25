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

#include "config.h"
#include "common.h"
#include "network.h"
#include "uart.h"
#include "enc28j60.h"
#include "timer.h"
#include "clock.h"
#include "sntp.h"
#include "eeprom.h"

#include "uip.h"
#include "uip_arp.h"

/* prototypes */
void init_spi(void);
void check_serial_input(uint8_t data);

void (*jump_to_bootloader)(void) = (void *)BOOTLOADER_SECTION;

/* macros */
#ifdef USE_WATCHDOG
#   define wdt_kick() wdt_reset()
#else
#   define wdt_kick()
#endif



void init_spi(void)
/* {{{ */ {

    /* configure MOSI, SCK, CS as outputs */
    SPI_DDR = _BV(SPI_MOSI) | _BV(SPI_SCK) | _BV(SPI_CS);
    DDRB |= _BV(PB0) | _BV(PB1);

    /* set all CS high (output) */
    SPI_PORT = _BV(SPI_CS);
    PORTB |= _BV(PB0) | _BV(PB1);

    /* enable spi, set master and clock modes (f/2) */
    _SPCR0 = _BV(_SPE0) | _BV(_MSTR0);
    _SPSR0 = _BV(_SPI2X0);

} /* }}} */

void check_serial_input(uint8_t data)
/* {{{ */ {

    switch (data) {

        case 'R': init_enc28j60();
                  break;

        case 'd': dump_debug_registers();
                  break;

        case 0x1b:  jump_to_bootloader();
                    break;

        case 'w':   /* test watchdog */
#                   ifdef USE_WATCHDOG
                    while (1);
#                   endif
                    break;

        case 's':   {
                        uart_puts_P("foo");
                        uip_ipaddr_t ip;
                        //uip_ipaddr(&ip, 134, 130, 4, 17);
                        uip_ipaddr(&ip, 137, 226, 147, 211);
                        //eeprom_load_ip(eeprom_config.sntp_server, &ip);
                        sntp_prepare_request(&ip);
                        break;
                    }

        case 't':   for (uint8_t i = 0; i < UIP_UDP_CONNS; i++) {
                        uart_puts_P("udp: local 0x");
                        uart_puthexbyte(LO8(uip_udp_conns[i].lport));
                        uart_puthexbyte(HI8(uip_udp_conns[i].lport));
                        uart_eol();
                    }
                    break;

        default:    uart_putc('?');
                    break;

    }

} /* }}} */

int main(void)
/* {{{ */ {

    /* initialize hardware */
    uart_init();

    /* enable interrupts */
    sei();

#   ifdef USE_WATCHDOG

#   ifdef DEBUG
    uart_puts_P("enabling watchdog\r\n");
#   endif

    /* test for reset cause */
    if (MCUSR & _BV(WDRF)) {
        uart_puts_P("BUG: got reset by the watchdog!!\r\n");

        /* clear flags */
        MCUSR &= ~_BV(WDRF);

#       ifdef DEBUG
        uart_puts_P("jumping to bootloader...\r\n");
        jump_to_bootloader();
#       endif

    }

    wdt_enable(WDTO_250MS);
    wdt_kick();
#   else
    uart_puts_P("disabling watchdog\r\n");
    wdt_disable();
#   endif

#   ifdef DEBUG
    /* send boot message */
    uart_puts_P("booting etherrape firmware " VERSION_STRING "...\r\n");
#   endif

    init_spi();
    timer_init();

    network_init();

#   ifdef DEBUG
    uart_puts_P("ip: ");
    uart_puts_ip(&uip_hostaddr);
    uart_eol();
#   endif

    while(1) /* main loop {{{ */ {

        wdt_kick();

        /* check for network controller interrupts */
        enc28j60_process_interrupts();

        /* check for timer interrupt */
        if (TIFR1 & _BV(OCF1A)) {

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
                    uip_arp_out();
                    transmit_packet();
                }
            }
#           endif

            if (c % 5 == 0) /* every second */
                clock_periodic();

            if (c == 50) { /* every 10 secs */
                uip_arp_timer();
                c = 0;
            }

            /* clear flag */
            TIFR1 = _BV(OCF1A);

        }

        wdt_kick();

        /* check for serial data */
        if (_UCSRA_UART0 & _BV(_RXC_UART0)) {

            check_serial_input(_UDR_UART0);

        }

        wdt_kick();

#       ifdef DEBUG
        /* if we changed to spi slave mode */
        if (!(SPCR0 & _BV(MSTR0))) {
            uart_puts_P("switched to spi slave mode? no pullup at SS?\r\n");

            SPCR0 |= _BV(MSTR0);

        }
#       endif

        wdt_kick();

    } /* }}} */

} /* }}} */
