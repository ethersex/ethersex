/*
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
 */

#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "usbdrv/usbdrv.h"
#include "protocols/ecmd/parser.h"
#include "requests.h"
#include "config.h"

#ifdef ECMD_USB_SUPPORT

#define ECMD_USB_BUFFER_LEN 50

static char recv_buffer[ECMD_USB_BUFFER_LEN + 1];
static uint8_t recv_count;

static char send_buffer[ECMD_USB_BUFFER_LEN + 1];
static uint8_t send_count;


uint8_t 
ecmd_usb_setup(uint8_t  data[8]) 
{
  usbRequest_t *rq = (void *)data;
  if (rq->wValue.word == 1) { /* Write */ 
    recv_count = 0;
  } else { /* Read */
    uint8_t len = ecmd_parse_command(recv_buffer, send_buffer, ECMD_USB_BUFFER_LEN);
    if (len > 0) 
      send_buffer[len] = 0;
    send_count = 0;
  }
  return USB_NO_MSG;
}

/* Host sends data to the device */
uint8_t
ecmd_usb_write(uint8_t *data, uint8_t len)
{
  if ((recv_count + len) > ECMD_USB_BUFFER_LEN) {
    len = ECMD_USB_BUFFER_LEN + 1 - recv_count;
    data[ECMD_USB_BUFFER_LEN] = 0;
  }

  memcpy(recv_buffer + recv_count, data, len);
  recv_count += len;

  if (recv_buffer[recv_count - 1] == 0)
    return 1;
  return 0;

}

/* Host requests data to the device */
uint8_t
ecmd_usb_read(uint8_t *data, uint8_t len)
{
  memcpy(data, send_buffer + send_count, len);
  send_count += len;
  return len;
}

#endif
