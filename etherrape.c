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
#include "74hc165.h"
#include "syslog.h"
#include "httpd.h"

#include "uip/uip.h"
#include "uip/uip_arp.h"

#include "onewire/onewire.h"

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

void print_rom_code(struct ow_rom_code_t *rom);
void print_rom_code(struct ow_rom_code_t *rom)
/* {{{ */ {

    uart_puthexbyte(rom->bytewise[0]);

    for (uint8_t j = 1; j < 8; j++) {
        uart_putc(' ');
        uart_puthexbyte(rom->bytewise[j]);
    }

} /* }}} */

void send_zero(void) {

    PORTD |= _BV(PD3);
    _delay_loop_2(2000);
    PORTD &= ~_BV(PD3);
    _delay_loop_2(2000);

}

void send_one(void) {

    PORTD |= _BV(PD3);
    _delay_loop_2(3000);
    PORTD &= ~_BV(PD3);
    _delay_loop_2(3000);

}

void on(void) {

    DDRD |= _BV(PD3);

    for (uint8_t j = 0; j < 3; j++) {

        /* sync */
        for (uint8_t i = 0; i < 12; i++)
            send_zero();
        send_one();

        /* hc1 */
        send_one();
        send_one();
        send_zero();
        send_one();
        send_zero();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_one();

        /* hc2 */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_one();
        send_zero();

        /* parity */
        send_one();

        /* address */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_zero();

        /* command */
        send_zero();
        send_zero();
        send_zero();
        send_one();
        send_zero();
        send_zero();
        send_zero();
        send_one();

        /* parity */
        send_zero();

        /* quersumme */
        send_one();
        send_one();
        send_one();
        send_zero();
        send_one();
        send_zero();
        send_zero();
        send_one();

        /* parity */
        send_one();

        /* eot */
        send_zero();

        _delay_loop_2(50000);
    }

    DDRD &= ~_BV(PD3);

    uart_puts_P("done\r\n");

}

void off(void) {
    DDRD |= _BV(PD3);

    for (uint8_t j = 0; j < 3; j++) {

        /* sync */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_one();

        /* hc1 */
        send_one();
        send_one();
        send_zero();
        send_one();
        send_zero();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_one();

        /* hc2 */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_one();
        send_zero();

        /* parity */
        send_one();

        /* address */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_zero();

        /* command */
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_zero();

        /* quersumme */
        send_one();
        send_one();
        send_zero();
        send_one();
        send_one();
        send_zero();
        send_zero();
        send_zero();

        /* parity */
        send_zero();

        /* eot */
        send_zero();

        _delay_loop_2(50000);
    }

    DDRD &= ~_BV(PD3);

    uart_puts_P("done\r\n");
}



void check_serial_input(uint8_t data)
/* {{{ */ {

    switch (data) {

#ifdef DEBUG
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

        case 't':   for (uint8_t i = 0; i < UIP_UDP_CONNS; i++) {
                        uart_puts_P("udp: local 0x");
                        uart_puthexbyte(LO8(uip_udp_conns[i].lport));
                        uart_puthexbyte(HI8(uip_udp_conns[i].lport));
                        uart_eol();
                    }
                    break;

        case 'p':
                    jump_to_bootloader();
                    break;

        case 'Y':   syslog_message_P("foobar?");
                    syslog_message_P("fump...");
                    break;
#endif

        case 'A':
                    on();
                    break;

        case 'a':
                    off();
                    break;

        case 'o':
                    {
                        struct ow_rom_code_t rom;
                        if (ow_read_rom(&rom) == 1) {
                            print_rom_code(&rom);
                            uart_eol();

                        } else
                            uart_puts_P("error reading rom code, CRC mismatch\r\n");

                        break;
                    }

        case 's':
                    {

                        /* initiate temperature convert cycle for all temperature sensors */
                        ow_temp_start_convert_nowait(NULL);
                        for (uint8_t t = 0; t < 46; t++)
                            _delay_loop_2(0);

                        int ret = ow_search_rom_first();

                        while (ret > 0) {

                            if (ow_temp_sensor(&ow_global.current_rom)) {
                                uart_puts_P("ow: found ");
                                print_rom_code(&ow_global.current_rom);
                                uart_puts_P(" temperature: ");

                                struct ow_temp_scratchpad_t scratchpad;
                                int8_t sp_ret = ow_temp_read_scratchpad(&ow_global.current_rom, &scratchpad);

                                if (sp_ret > 0) {

                                    int16_t temp = ow_temp_normalize(&ow_global.current_rom, &scratchpad);

                                    if (temp < 0)
                                        uart_putc('-');

                                    uart_putdecbyte(HIGH(temp));

                                    uart_putc('.');

                                    if (LOW(temp))
                                        uart_putc('5');
                                    else
                                        uart_putc('0');

                                    uart_eol();

                                } else if (sp_ret == -3)
                                    uart_puts_P("no temperature sensor...\r\n");
                                else if (sp_ret == -2)
                                    uart_puts_P("Error, CRC mismatch\r\n");
                                else
                                    uart_puts_P("no device detected\r\n");
                            } else if (ow_global.current_rom.family == OW_FAMILY_DS2502E48) {
                                uart_puts_P("ow: found mac address: ");
                                print_rom_code(&ow_global.current_rom);
                                uart_eol();

                                uint8_t mac[6];
                                int8_t ret_data = ow_eeprom_read(&ow_global.current_rom, &mac);

                                if (ret_data == 0) {

                                    uart_puts_P(" mac data: ");

                                    uart_puts_mac((struct uip_eth_addr *)&mac);

                                    uart_eol();

                                } else if (ret_data == -1) {
                                    uart_puts_P("ow: no device??\r\n");
                                } else if (ret_data == -2) {
                                    uart_puts_P("ow: crc error\r\n");
                                }

                            } else {
                                uart_puts_P("ow: unknown device: ");
                                print_rom_code(&ow_global.current_rom);
                                uart_eol();
                            }

                            ret = ow_search_rom_next();
                        }

                        break;
                    }

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

    //wdt_enable(WDTO_250MS);
    wdt_enable(WDTO_2S); // temp, for onewire debugging
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
    syslog_init();

    network_init();

#   ifdef DEBUG
    uart_puts_P("ip: ");
    uart_puts_ip(&uip_hostaddr);
    uart_eol();
#   endif

#   ifdef USE_74HC165
    hc165_init();
    DDRA |= _BV(PA4);
#   endif

#   ifdef ONEWIRE_SUPPORT
    init_onewire();
#   endif

    /* HACKHACKHACK */
    DDRD &= ~_BV(PD3);
    PORTD &= ~_BV(PD3);

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
                    if (uip_arp_out() == 0)
                        uip_udp_conn->appstate.sntp.transmit_state = 1;

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
            _TIFR_TIMER1 = _BV(OCF1A);

        }

        wdt_kick();

        /* check for serial data */
        if (_UCSRA_UART0 & _BV(_RXC_UART0)) {

            check_serial_input(_UDR_UART0);

        }

        wdt_kick();

#       ifdef DEBUG
        /* if we changed to spi slave mode */
        if (!(_SPCR0 & _BV(_MSTR0))) {
            uart_puts_P("switched to spi slave mode? no pullup at SS?\r\n");

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
