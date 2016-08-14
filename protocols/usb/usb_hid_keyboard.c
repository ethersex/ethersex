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
#include "usb_hid_keyboard.h"
#include "usbdrv/usbdrv.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef USB_KEYBOARD_SUPPORT
#include "usb_hid_keyboard_keymap_default.h"

#define MAX_SEND_LEN 64

uchar lastKey = 0, keyDidChange = 0;

uchar reportBuffer[2];    /* buffer for HID reports */
uchar idleRate;           /* in 4 ms units */
uchar key;

char *send_buf;
uint8_t send_pos=0;
#ifdef USB_HID_KEYBOARD_STATIC_SUPPORT
PROGMEM const char usb_text[]= USB_HID_KEYBOARD_TEXT;
#endif

PROGMEM const char usbHidReportDescriptor[35] = { /* USB report descriptor */
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

uchar keyPressed(void)
{
#ifdef USB_HID_KEYBOARD_SEQUENCE_SUPPORT
    if (key >= 'z')
      key = 'a';
    else
      key++;
    keyDidChange = 1;
#endif
#ifdef USB_HID_KEYBOARD_STATIC_SUPPORT
    key = pgm_read_byte(&usb_text[send_pos]);
	USBKEYBOARDDEBUG("buffer pos %i, char: %c, key: %c\n",send_pos, usb_text[send_pos], key);
    keyDidChange = 1;
    send_pos++;
    if (send_pos >= sizeof(usb_text) -1 ) send_pos=0;
#endif
#ifdef USB_HID_KEYBOARD_ECMD_SUPPORT
    if (send_buf != 0) { // if buffer is set do... 
    // get key from buffer and set key
      key = send_buf[send_pos++];
      if (key == 0x0 || send_pos >= MAX_SEND_LEN) {  // buffer finished, clear!
        send_buf=0;
        return lastKey;
      }
      keyDidChange = 1;
    }
#endif
	return key;
}

#define HID_KEYMAP_SIZE (sizeof(keyReport) / sizeof(struct hid_keyboard_map_t))
void buildReport(uchar key)
{
/* This (not so elegant) cast saves us 10 bytes of program memory */
//    *(int *)reportBuffer = pgm_read_word(keyReport[key]);
// modifier with ascii key table

    char i;
    struct hid_keyboard_map_t keymapentry; 

    for (i=0;i < HID_KEYMAP_SIZE;i++) {
      memcpy_P(&keymapentry, &keyReport[i], sizeof(struct hid_keyboard_map_t));
      if (key == keymapentry.character) {
	    memcpy(reportBuffer, keymapentry.reportBuffer, sizeof(reportBuffer));
	    return;
      }
    }
    // char not found
    reportBuffer[0] = MOD_NONE;
    reportBuffer[1] = 0x0;
}

void
usb_keyboard_periodic(void) {
	if(keyDidChange && usbInterruptIsReady()){
		USBKEYBOARDDEBUG("usb set interrupt lastkey %c, key: %i\n",lastKey, key);
        buildReport(lastKey);
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
	}
	// send end of key
	while (keyDidChange && !usbInterruptIsReady()){
		_delay_ms(10); // as slow speed device we have to wait until next call
		usbPoll();
		buildReport(0);
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		keyDidChange = 0;
	}
}
void
usb_keyboard_periodic_call (void)
{
	USBKEYBOARDDEBUG("pre  key pressed: %i, last %i, change: %i\n",key, lastKey, keyDidChange);
        key = keyPressed();
        if(lastKey != key){
			lastKey = key;
        	keyDidChange = 1;
        }
	USBKEYBOARDDEBUG("post key pressed: %i, last %i, change: %i\n",key, lastKey, keyDidChange);
}


void
usb_keyboard_init(void) {
#ifdef USB_HID_KEYBOARD_SEQUENCE_SUPPORT
  key='a';
#endif
}

#ifdef USB_HID_KEYBOARD_ECMD_SUPPORT
uint8_t
send_string (char *str) {

  if (send_buf) return 0;

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
  send_pos=0;

  return 1;
}

int16_t parse_cmd_keyboard_send (char *cmd, char *output, uint16_t len) 
{
	send_string(cmd);
	return ECMD_FINAL_OK;
}
#endif /* USB_HID_KEYBOARD_ECMD_SUPPORT */

uint16_t
hid_usbFunctionSetup(uchar data[8]) 
{
  usbRequest_t    *rq = (void *)data;
  USBKEYBOARDDEBUG("setup keyboard\n");
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

/*
  -- Ethersex META --
  header(protocols/usb/usb_hid_keyboard.h)
  init(usb_keyboard_init)
  timer(8,usb_keyboard_periodic_call())
  block([[USB]])
  ecmd_ifdef(USB_HID_KEYBOARD_ECMD_SUPPORT)
    ecmd_feature(keyboard_send, "keyboard ",MESSAGE,Send MESSAGE as HID keyboard)
  ecmd_endif()
*/

#endif
