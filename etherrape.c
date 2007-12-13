/* vim:fdm=marker et ai
 * {{{
 *
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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
#include "uip/uip_neighbor.h"

#include "config.h"
#include "global.h"
#include "debug.h"
#include "spi.h"
#include "network.h"
#include "timer.h"
#include "fs20/fs20.h"
#include "lcd/hd44780.h"
#include "watchcat/watchcat.h"
#include "onewire/onewire.h"
#include "rc5/rc5.h"
#include "rfm12/rfm12.h"
#include "ipv6.h"
#include "sensor_rfm12/sensor_rfm12.h"

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
#   ifdef BOOTLOADER_SUPPORT
    _IVREG = _BV(IVCE);	            /* prepare ivec change */
    _IVREG = _BV(IVSEL);            /* change ivec to bootloader */
#   endif

    debug_init();
    debug_printf("debugging enabled\n");

#   ifdef HD44780_SUPPORT
    debug_printf("initializing lcd...\n");
    hd44780_init(0, 0);
#   ifdef DEBUG
    fprintf_P(lcd, PSTR("booting...\n"));
#   endif
#   endif

#   ifdef WATCHCAT_SUPPORT
    watchcat_init();
#   endif

    /* enable interrupts */
    sei();

#   ifdef USE_WATCHDOG
    debug_printf("enabling watchdog\n");

#   ifdef DEBUG
    /* for debugging, test reset cause and jump to bootloader */
    if (MCUSR & _BV(WDRF)) {
        debug_printf("BUG: got reset by the watchdog!!\n");

        /* clear flags */
        MCUSR &= ~_BV(WDRF);

        debug_printf("jumping to bootloader...\n");
        jump_to_bootloader();

    }
#   endif

    /* set watchdog to 2 seconds */
    wdt_enable(WDTO_2S);
    wdt_kick();
#   else
    debug_printf("disabling watchdog\n");
    wdt_disable();
#   endif

    /* send boot message */
    debug_printf("booting etherrape firmware " VERSION_STRING "...\n");

    spi_init();
    network_init();
    timer_init();
#ifdef CLOCK_SUPPORT
    clock_init();
#endif

#ifdef FS20_SUPPORT
    fs20_init();
#endif

#ifdef ONEWIRE_SUPPORT
    onewire_init();
#endif

#ifdef RC5_SUPPORT
    rc5_init();
#endif

#ifdef RFM12_SUPPORT
    rfm12_init();

#ifdef TEENSY_SUPPORT
    cli ();
    rfm12_trans (0xa620);	/* rfm12_setfreq(RFM12FREQ(433.92)); */
    rfm12_trans (0x94ac);	/* rfm12_setbandwidth(5, 1, 4); */
    rfm12_trans (0xc609);	/* rfm12_setbaud(34482); */
    rfm12_trans (0x9820);	/* rfm12_setpower(0, 2); */
    sei ();
#else
    rfm12_setfreq(RFM12FREQ(433.92));
    rfm12_setbandwidth(5, 1, 4);
    rfm12_setbaud(34482);
    rfm12_setpower(0, 2);
#endif

    rfm12_rxstart();
#endif

    /* must be called AFTER all other initialization */
#ifdef PORTIO_SUPPORT
    portio_init();
#endif 

    debug_printf("enc28j60 revision 0x%x\n", read_control_register(REG_EREVID));
    debug_printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
            uip_ethaddr.addr[0],
            uip_ethaddr.addr[1],
            uip_ethaddr.addr[2],
            uip_ethaddr.addr[3],
            uip_ethaddr.addr[4],
            uip_ethaddr.addr[5]
            );

#   if defined(HD44780_SUPPORT) && defined(DEBUG)
    fprintf_P(lcd, PSTR("mac: %02x%02x%02x%02x%02x%02x\n"),
            uip_ethaddr.addr[0], uip_ethaddr.addr[1],
            uip_ethaddr.addr[2], uip_ethaddr.addr[3],
            uip_ethaddr.addr[4], uip_ethaddr.addr[5]
            );
#   endif

    cfg.request_reset = 0;
    cfg.request_bootloader = 0;

    /* main loop */
    while(1) {

        wdt_kick();

#ifdef ENC28J60_SUPPORT
        /* check for network controller interrupts,
         * call uip on received packets */
        network_process();
        wdt_kick();
#endif

#ifdef RFM12_SUPPORT
	rfm12_process();
	wdt_kick();
#endif

        /* check if any timer expired,
         * poll all uip connections */
        timer_process();
        wdt_kick();

#ifdef PORTIO_SUPPORT
        /* update port io information */
        portio_update();
        wdt_kick();
#endif

        /* check if debug input has arrived */
        debug_process();
        wdt_kick();

        /* check if fs20 data has arrived */
#ifdef FS20_SUPPORT
#ifdef FS20_SUPPORT_RECEIVE
        fs20_process();
        wdt_kick();
#endif

        /* check if rc5 data has arrived */
#ifdef RC5_SUPPORT
        rc5_process();
        wdt_kick();
#endif
#endif /* FS20_SUPPORT */

#       ifdef STELLA_SUPPORT
	stella_timer ();
#       endif

#ifndef BOOTLOAD_SUPPORT
        if(cfg.request_bootloader) {
        #ifdef CLOCK_CRYSTAL_SUPPORT
            TIMSK2 &= ~_BV(TOIE2);
        #endif
            cli();
            jump_to_bootloader();
        }

        if(cfg.request_reset) {
            cli();

            void (* reset)(void) = NULL;
            reset();
        }
#endif
    }

} /* }}} */
