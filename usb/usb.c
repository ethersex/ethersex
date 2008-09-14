/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "../bit-macros.h"
#include "../uip/uip.h"
#include "usbdrv/usbdrv.c"


#ifdef USB_SUPPORT

#ifdef USB_CFG_PULLUP_IOPORTNAME
#undef usbDeviceConnect
#define usbDeviceConnect()      do { \
                                    USB_PULLUP_DDR |= (1<<USB_CFG_PULLUP_BIT); \
                                    USB_PULLUP_OUT |= (1<<USB_CFG_PULLUP_BIT); \
                                    USB_INTR_ENABLE |= (1 << USB_INTR_ENABLE_BIT); \
                                   } while(0);
#undef usbDeviceDisconnect
#define usbDeviceDisconnect()   do { \
                                    USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT); \
                                    USB_PULLUP_DDR &= ~(1<<USB_CFG_PULLUP_BIT); \
                                    USB_PULLUP_OUT &= ~(1<<USB_CFG_PULLUP_BIT); \
                                   } while(0);

#endif


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  return 0;   /* default for not implemented requests: return no data back to host */
}


/* Wrapper functions to integrate the usb driver in ethersex */

void
usb_periodic(void)
{
  usbPoll();
  //if(usbInterruptIsReady()){
    /* called after every poll of the interrupt endpoint */
  //}
}

void 
usb_init(void) 
{
#define USB_DDR_CONFIG(pin)  DDR_CHAR( pin ## _PORT) &= ~(_BV((pin ## _PIN)) | _BV(USB_INT_PIN))
#define USB_PORT_CONFIG(pin)  PORT_CHAR( pin ## _PORT) &= ~(_BV((pin ## _PIN)) | _BV(USB_INT_PIN))
  USB_DDR_CONFIG(USB_DMINUS);
  USB_PORT_CONFIG(USB_DMINUS);
#undef USB_DDR_CONFIG
#undef USB_PORT_CONFIG
  
  uint8_t i;
  /* Reenummerate the device */
  usbDeviceDisconnect();
  for(i = 0; i < 20; i++){  /* 300 ms disconnect */
    _delay_ms(15);
    wdt_kick();
  }
  usbDeviceConnect();

  /* USB Initialize */
  usbInit();
}

#endif
