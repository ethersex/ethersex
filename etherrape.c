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
#include "timer.h"

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
    timer_init();

    debug_printf("enc28j60 revision 0x%x\n", read_control_register(REG_EREVID));
    debug_printf("ip: %d.%d.%d.%d\n", LO8(uip_hostaddr[0]),
                                      HI8(uip_hostaddr[0]),
                                      LO8(uip_hostaddr[1]),
                                      HI8(uip_hostaddr[1]));

    /* main loop */
    while(1) {

        wdt_kick();

        /* check for network controller interrupts */
        network_process();
        wdt_kick();

        /* check if any timer expired */
        timer_process();
        wdt_kick();

        /* update port io information */
        portio_update();
        wdt_kick();

    }

} /* }}} */
