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

#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/uip_neighbor.h"

#include "config.h"
#include "global.h"
#include "core/debug.h"
#include "core/spi.h"
#include "network.h"
#include "core/periodic.h"
#include "core/portio/portio.h"
#include "hardware/radio/fs20/fs20.h"
#include "hardware/lcd/hd44780.h"
#include "watchcat/watchcat.h"
#include "control6/control6.h"
#include "onewire/onewire.h"
#include "ecmd_serial/ecmd_serial_i2c.h"
#include "ecmd_serial/ecmd_serial_usart.h"
#include "rc5/rc5.h"
#include "hardware/radio/rfm12/rfm12.h"
#include "dcf77/dcf77.h"
#include "ps2/ps2.h"
#include "hc165/hc165.h"
#include "hc595/hc595.h"
#include "ipv6.h"
#include "hardware/storage/dataflash/fs.h"
#include "services/clock/clock.h"
#include "services/cron/cron.h"
#include "services/stella/stella.h"
#include "protocols/modbus/modbus.h"
#include "protocols/zbus/zbus.h"
#include "protocols/usb/usb.h"
#include "protocols/yport/yport.h"
#include "syslog/syslog.h"
#include "net/handler.h"
#include "net/sendmail.h"
#include "hardware/camera/dc3840.h"
#include "hardware/storage/sd_reader/sd_raw.h"
#include "i2c_master/i2c_master.h"
#include "i2c_master/i2c_24CXX.h"
#include "i2c_master/i2c_lm75.h"
#include "vfs/vfs.h"

#include "core/bit-macros.h"

/* global configuration */
global_status_t status;

/* prototypes */
void (*jump_to_bootloader)(void) = (void *)BOOTLOADER_SECTION;

int main(void)
{
#   ifdef BOOTLOADER_SUPPORT
    _IVREG = _BV(IVCE);	            /* prepare ivec change */
    _IVREG = _BV(IVSEL);            /* change ivec to bootloader */
#   endif

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

#   ifdef BOOTLOADER_SUPPORT
    /* disable interrupts */
    cli();
#   else
    /* enable interrupts */
    sei();
#   endif

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
    debug_printf("booting ethersex firmware " VERSION_STRING "...\n");

#   if defined(RFM12_SUPPORT) || defined(ENC28J60_SUPPORT) \
      || defined(DATAFLASH_SUPPORT)
    spi_init();
#   endif

#   ifdef DATAFLASH_SUPPORT
    debug_printf("initializing filesystem...\n");
    fs_init(&fs, NULL);
    debug_printf("fs: root page is 0x%04x\n", fs.root);
#   endif

#   ifdef CRON_SUPPORT
    cron_init();
#   endif

#   ifdef UIP_SUPPORT
    network_init();
#   endif

    periodic_init();

#ifdef CLOCK_SUPPORT
    clock_init();
#endif

#ifdef ADC_SUPPORT
    /* ADC Prescaler to 64 */
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);
    /* ADC set Voltage Reference to extern*/
    /* FIXMI: the config to the right place */
    ADMUX = ADC_REF; //_BV(REFS0) | _BV(REFS1);
#endif

#ifdef PS2_SUPPORT
    ps2_init();
#endif

#ifdef DCF77_SUPPORT
    dcf77_init();
#endif

/* The I2C Master Stuff */

#ifdef I2C_MASTER_SUPPORT
    i2c_master_init();
#endif

#ifdef I2C_24CXX_SUPPORT
    i2c_24CXX_init();
#endif

#ifdef VFS_EEPROM_SUPPORT
    vfs_eeprom_init();
#endif

/* End of the I2C Master Stuff */

#ifdef USB_SUPPORT
    usb_init();
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

#ifdef STELLA_SUPPORT
	stella_init();
#endif

/* Had to be bone after network_init! */
#ifdef YPORT_SUPPORT
    yport_init();
#endif

#ifdef MODBUS_SUPPORT
    modbus_init();
#endif

#ifdef ECMD_SERIAL_I2C_SUPPORT
    ecmd_serial_i2c_init();
#endif

#ifdef ECMD_SERIAL_USART_SUPPORT
    ecmd_serial_usart_init();
#endif

#ifdef ZBUS_SUPPORT
    zbus_core_init();
#endif

#ifdef HC595_SUPPORT
    hc595_init();
#endif

#ifdef HC165_SUPPORT
    hc165_init();
#endif

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

#ifdef DC3840_SUPPORT
    dc3840_init ();
#endif	/* DC3840_SUPPORT */

    /* must be called AFTER all other initialization */
#ifdef PORTIO_SUPPORT
    portio_init();
#elif defined(NAMED_PIN_SUPPORT)
    np_simple_init();
#endif

#ifdef CONTROL6_SUPPORT
    control6_init();
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

#   if defined(HD44780_SUPPORT) && defined(DEBUG)
    fprintf_P(lcd, PSTR("mac: %02x%02x%02x%02x%02x%02x\n"),
            uip_ethaddr.addr[0], uip_ethaddr.addr[1],
            uip_ethaddr.addr[2], uip_ethaddr.addr[3],
            uip_ethaddr.addr[4], uip_ethaddr.addr[5]
            );
#   endif

    status.request_reset = 0;
    status.request_bootloader = 0;

#ifdef SENDMAIL_SUPPORT
    mail_send ();
#endif

    /* main loop */
    while(1) {

        wdt_kick();

#ifdef ENC28J60_SUPPORT
        /* check for network controller interrupts,
         * call uip on received packets */
        network_process();
        wdt_kick();
#endif

#ifdef RFM12_IP_SUPPORT
	rfm12_process();
	wdt_kick();
#endif

#ifdef ZBUS_SUPPORT
	zbus_process();
	wdt_kick();
#endif

#       ifdef USB_SUPPORT
        usb_periodic();
	wdt_kick();
#       endif

#       ifdef SD_READER_SUPPORT
	if (sd_active_partition == NULL) {
	    if (! sd_try_init ())
		vfs_sd_try_open_rootnode ();

	    wdt_kick();
	}
#       endif

#ifdef STELLA_SUPPORT
stella_process();
wdt_kick();
#endif

        /* check if any timer expired,
         * poll all uip connections */
		  periodic_process();
        wdt_kick();

        /* check if debug input has arrived */
        debug_process();
        wdt_kick();

#ifdef SYSLOG_SUPPORT
	uip_stack_set_active(STACK_ENC);
        syslog_flush();
#endif

        /* check if fs20 data has arrived */
#if defined(FS20_SUPPORT) && defined(FS20_RECEIVE_SUPPORT)
        fs20_process();
        wdt_kick();
#endif  /* FS20_SUPPORT && FS20_RECEIVE_SUPPORT */

        /* check if rc5 data has arrived */
#ifdef RC5_SUPPORT
        rc5_process();
        wdt_kick();
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
