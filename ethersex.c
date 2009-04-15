/*
 * Copyright (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "protocols/uip/uip.h"
#include "protocols/uip/uip_arp.h"
#include "protocols/uip/uip_neighbor.h"

#include "config.h"
#include "core/global.h"
#include "core/debug.h"
#include "core/spi.h"
#include "network.h"
#include "core/periodic.h"
#include "core/portio/portio.h"
#include "hardware/radio/rfm12/rfm12.h"
#include "protocols/uip/ipv6.h"
#include "hardware/storage/sd_reader/sd_raw.h"
#include "core/vfs/vfs.h"

#include "core/bit-macros.h"

/* global configuration */
global_status_t status;

/* prototypes */
void (*jump_to_bootloader)(void) = (void *)BOOTLOADER_SECTION;


extern void ethersex_meta_init(void);
extern void ethersex_meta_startup(void);
extern void ethersex_meta_mainloop(void);
extern void periodic_process(void);

int main(void)
{
	#ifdef STATUSLED_POWER
	PIN_SET(STATUSLED_POWER);
	#endif

	#ifdef BOOTLOADER_SUPPORT
	_IVREG = _BV(IVCE);	            /* prepare ivec change */
	_IVREG = _BV(IVSEL);            /* change ivec to bootloader */
	#endif

	/* Clear the MCUSR Register to avoid endless wdreset loops */
	#ifdef MCUSR
	MCUSR = 0;
	#endif
	#ifdef MCUCSR
	MCUCSR = 0;
	#endif

	/* Default DDR Config */
	#if IO_HARD_PORTS == 4 && DDR_MASK_A != 0
	DDRA = DDR_MASK_A;
	#endif
	#if DDR_MASK_B != 0
	DDRB = DDR_MASK_B;
	#endif
	#if DDR_MASK_C != 0
	DDRC = DDR_MASK_C;
	#endif
	#if DDR_MASK_D != 0
	DDRD = DDR_MASK_D;
	#endif
	#if IO_HARD_PORTS == 6
		#if DDR_MASK_E != 0
		DDRE = DDR_MASK_E;
		#endif
		#if DDR_MASK_F != 0
		DDRF = DDR_MASK_F;
		#endif
	#endif

	debug_printf("debugging enabled\n");

	#ifdef BOOTLOADER_SUPPORT
	/* disable interrupts */
	cli();
	#else
	/* enable interrupts */
	sei();
	#endif //BOOTLOADER_SUPPORT

	#ifdef USE_WATCHDOG
	debug_printf("enabling watchdog\n");

		#ifdef DEBUG
		/* for debugging, test reset cause and jump to bootloader */
		if (MCUSR & _BV(WDRF)) {
			debug_printf("BUG: got reset by the watchdog!!\n");

			/* clear flags */
			MCUSR &= ~_BV(WDRF);

			debug_printf("jumping to bootloader...\n");
			jump_to_bootloader();

		}
		#endif

	/* set watchdog to 2 seconds */
	wdt_enable(WDTO_2S);
	wdt_kick();
	#else //USE_WATCHDOG
	debug_printf("disabling watchdog\n");
	wdt_disable();
	#endif //USE_WATCHDOG

	/* send boot message */
	debug_printf("booting ethersex firmware " VERSION_STRING "...\n");

	#if defined(RFM12_SUPPORT) || defined(ENC28J60_SUPPORT) \
      || defined(DATAFLASH_SUPPORT)
    spi_init();
	#endif

	#ifdef ADC_SUPPORT
	/* ADC Prescaler to 64 */
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
	/* ADC set Voltage Reference to extern*/
	/* FIXMI: the config to the right place */
	ADMUX = ADC_REF; //_BV(REFS0) | _BV(REFS1);
	#endif

	ethersex_meta_init();

	#ifdef RFM12_SUPPORT
		rfm12_init();

		#ifdef TEENSY_SUPPORT
		cli ();
		rfm12_trans (0xa620);	/* rfm12_setfreq(RFM12FREQ(433.92)); */
		rfm12_trans (0x94ac);	/* rfm12_setbandwidth(5, 1, 4); */
			#ifdef RFM12_IP_SUPPORT
			rfm12_trans (0xc610);	/* rfm12_setbaud(192); */
			rfm12_trans (0x9820);	/* rfm12_setpower(0, 2); */
			rfm12_rxstart();
			#endif  /* RFM12_IP_SUPPORT */
		sei ();
		#else  /* TEENSY_SUPPORT */
		rfm12_setfreq(RFM12FREQ(433.92));
		rfm12_setbandwidth(5, 1, 4);
			#ifdef RFM12_IP_SUPPORT
			rfm12_setbaud(CONF_RFM12_BAUD / 100);
			rfm12_setpower(0, 2);
			rfm12_rxstart();
			#endif  /* RFM12_IP_SUPPORT */
		#endif  /* not TEENSY_SUPPORT */
	#endif  /* RFM12_SUPPORT */

	/* must be called AFTER all other initialization */
	#ifdef PORTIO_SUPPORT
	portio_init();
	#elif defined(NAMED_PIN_SUPPORT)
	np_simple_init();
	#endif

	#ifdef ENC28J60_SUPPORT
	debug_printf("enc28j60 revision 0x%x\n", read_control_register(REG_EREVID));
	debug_printf("mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
			uip_ethaddr.addr[0],
			uip_ethaddr.addr[1],
			uip_ethaddr.addr[2],
			uip_ethaddr.addr[3],
			uip_ethaddr.addr[4],
			uip_ethaddr.addr[5]
			);
	#endif

	#ifdef STATUSLED_BOOTED
	PIN_SET(STATUSLED_BOOTED);
	#endif

	ethersex_meta_startup();

	/* main loop */
	while(1) {

	wdt_kick();
	ethersex_meta_mainloop();

	#ifdef SD_READER_SUPPORT
	if (sd_active_partition == NULL) {
	    if (! sd_try_init ())
		vfs_sd_try_open_rootnode ();

	    wdt_kick();
	}
	#endif

	#ifndef BOOTLOAD_SUPPORT
		if(status.request_bootloader) {
			#ifdef CLOCK_CRYSTAL_SUPPORT
			TIMSK2 &= ~_BV(TOIE2);
			#endif
			#ifdef DCF77_SUPPORT
			ACSR &= ~_BV(ACIE);
			#endif
			cli();
			jump_to_bootloader();
		}

		#ifndef TEENSY_SUPPORT
		if(status.request_wdreset) {
			cli();
			wdt_enable(WDTO_15MS);
			for(;;);
		}
		#endif

		if(status.request_reset) {
			cli();

			void (* reset)(void) = NULL;
			reset();
		}
	#endif
	}

}
