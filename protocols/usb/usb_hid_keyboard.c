/*
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
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
 */

#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "requests.h"
#include "usbdrv/usbdrv.h"
#include "usb_hid_keyboard.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef USB_KEYBOARD_SUPPORT

#define MAX_SEND_LEN 64
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

uchar key, lastKey = 0, keyDidChange = 0;

char *send_buf;
uint8_t send_pos=0;
uint8_t start_send=0;

static uchar keyPressed(void)
{
	key++;
	if (key>15) key=0;
	/*	future use
	if (start_send) {
		key=send_buf[send_pos];
		USBKEYBOARDDEBUG("buffer pos %i, char: %c\n",send_pos, key);
		if (key == 0 || (send_pos > MAX_SEND_LEN) ) {
			// end erreicht
			send_pos = 0;
			start_send=0;  
		} else {  
			send_pos++;
		} 
	} else {
		return 0;
	}
	*/
	return key;
}



PROGMEM char usbHidReportDescriptor[35] = { /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};



static const char  keyReport[NUM_KEYS + 1][2] PROGMEM = {
/* none */  {0, 0},                     /* no key pressed */
/*  1 */    {MOD_NONE, KEY_A},
/*  2 */    {MOD_NONE, KEY_B},
/*  3 */    {MOD_NONE, KEY_C},
/*  4 */    {MOD_NONE, KEY_D},
/*  5 */    {MOD_NONE, KEY_E},
/*  6 */    {MOD_NONE, KEY_F},
/*  7 */    {MOD_NONE, KEY_G},
/*  8 */    {MOD_NONE, KEY_H},
/*  9 */    {MOD_NONE, KEY_I},
/* 10 */    {MOD_NONE, KEY_J},
/* 11 */    {MOD_NONE, KEY_K},
/* 12 */    {MOD_NONE, KEY_L},
/* 13 */    {MOD_NONE, KEY_M},
/* 14 */    {MOD_NONE, KEY_N},
/* 15 */    {MOD_NONE, KEY_O},
/* 16 */    {MOD_NONE, KEY_P},
/* 17 */    {MOD_NONE, KEY_Q},
};


void buildReport(uchar key)
{
/* This (not so elegant) cast saves us 10 bytes of program memory */
    *(int *)reportBuffer = pgm_read_word(keyReport[key]);
}

void
usbFunctionReadFinished(void)
{
}

usbMsgLen_t	usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            buildReport(keyPressed());
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
	return 0;
}

void
usb_keyboard_init (void)
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

	USBKEYBOARDDEBUG("init\n");


}

uchar reportBuffer[2];    /* buffer for HID reports */
uchar idleRate=1;           /* in 4 ms units */

void
usb_keyboard_periodic (void)
{
	usbPoll();
	if(keyDidChange && usbInterruptIsReady()){
		USBKEYBOARDDEBUG("usb set interrupt lastkey %c, key: %i\n",lastKey, key);
            	buildReport(lastKey);
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
	}
	// send end of key
	while (keyDidChange && !usbInterruptIsReady()) {
	_delay_ms(10); // as slow speed device we have to wait until next call
//		USBKEYBOARDDEBUG("try stop\n");
		usbPoll();
            	buildReport(0);
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		keyDidChange = 0;
	}
}

void
usb_keyboard_periodic_call (void)
{
        key = keyPressed();
        if(lastKey != key){
		lastKey = key;
        	keyDidChange = 1;
        }
//	USBKEYBOARDDEBUG("key pressed: %i\n",key);
}


uint8_t
send_string (char *str) {

  if (send_buf) return 0;

  start_send=1;

  uint8_t len = strlen(str);
  if (len > MAX_SEND_LEN) {
	USBKEYBOARDDEBUG("message to long: cut\n");
	len=MAX_SEND_LEN;
  }
  send_buf = malloc(MAX_SEND_LEN);
  if (!send_buf) return 0;

  memcpy(send_buf, str, len);
  send_buf[len] = 0;
  send_buf[MAX_SEND_LEN] = 0;

  return 1;
}

int16_t parse_cmd_keyboard_send (char *cmd, char *output, uint16_t len) 
{
	send_string(cmd);
	return ECMD_FINAL_OK;
}
/*
  -- Ethersex META --
  header(protocols/usb/usb_hid_keyboard.h)
  init(usb_keyboard_init)
  timer(1,usb_keyboard_periodic())
  timer(250,usb_keyboard_periodic_call())
  ecmd_feature(keyboard_send, "keyboard ",MESSAGE,Send MESSAGE as HID keyboard)
*/

#endif
