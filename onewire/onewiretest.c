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
#include <util/delay.h>
#include <util/delay.h>
#include <util/crc16.h>
#include "../config.h"
#include "../uart.h"
#include "../fifo.h"
#include "../crc.h"
#include "onewire.h"

/* prototypes */
void check_serial_input(uint8_t data);
void init_system_timer(void);
void print_rom_code(struct ow_rom_code_t *rom);

void (*jump_to_bootloader)(void) = (void *)BOOTLOADER_SECTION;



void print_rom_code(struct ow_rom_code_t *rom)
/* {{{ */ {

    uart_puthexbyte(rom->bytewise[0]);

    for (uint8_t j = 1; j < 8; j++) {
        uart_putc(' ');
        uart_puthexbyte(rom->bytewise[j]);
    }

} /* }}} */

void check_serial_input(uint8_t data)
/* {{{ */ {
    switch (data) {
        case 0x1b:
                    jump_to_bootloader();
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
                        struct ow_rom_code_t roms[10];

                        int8_t ret = ow_search_rom(roms, 10);

                        _delay_loop_2(65534);

                        uart_puts_P("discovered ");
                        uart_puthexbyte(ret);
                        uart_puts_P(" roms codes\r\n");

                        _delay_loop_2(65534);

                        for (uint8_t i = 0; i < ret; i++) {
                            uart_putc(' ');
                            uart_putdecbyte(i);
                            uart_puts_P(":  ");
                            print_rom_code(&roms[i]);
                            uart_eol();
                        }

                        uart_puts_P("starting temperature conversion\r\n");
#                       ifdef ONEWIRE_PARASITE
                            ow_start_convert(NULL, 0);
                            for (uint8_t t = 0; t < 46; t++)
                                _delay_loop_2(0);
#                       else
                            ow_start_convert(NULL, 1);
#                       endif

                        uart_puts_P("temperatures:\r\n");
                        for (uint8_t i = 0; i < ret; i++) {

                            struct ow_scratchpad_t scratchpad;

                            if (ow_read_scratchpad(&roms[i], &scratchpad) > 0) {

                                uart_putc(' ');
                                uart_putdecbyte(i);
                                uart_puts_P(":  ");

                                if (scratchpad.temperature_high & 0x80)
                                    uart_putc('-');

                                uart_putdecbyte(scratchpad.temperature_low >> 1);

                                uart_putc('.');

                                if (scratchpad.temperature_low & 0x01)
                                    uart_putc('5');
                                else
                                    uart_putc('0');

                                uart_eol();

                            } else
                                uart_puts_P("Error, CRC mismatch\r\n");

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
    init_uart();

    /* enable interrupts */
    sei();

    /* send boot message */
    uart_putc('B');

    init_onewire();

    while(1) /* main loop {{{ */ {

        /* check and process uart receive fifo */
        if (fifo_fill(&global_uart.rx_fifo) > 0) {
            check_serial_input(fifo_load(&global_uart.rx_fifo));
        }

    } /* }}} */
} /* }}} */
