/*
 *
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef _USB_REQUESTS_H
#define _USB_REQUESTS_H

enum {
  /* For USB Ecmd */
  USB_REQUEST_ECMD = 0,

  /* USB networking */
  USB_REQUEST_NET_SEND = 10,
  USB_REQUEST_NET_RECV = 11,
};


uint8_t ecmd_usb_setup(uint8_t  data[8]);
uint8_t ecmd_usb_write(uint8_t *data, uint8_t len);
uint8_t ecmd_usb_read(uint8_t *data, uint8_t len);

#endif /* _USB_REQUESTS_H */
