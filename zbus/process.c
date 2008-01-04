/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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

#include "../config.h"
#include "../uip/uip.h"
#include "zbus.h"


void
zbus_process(void)
{
  uip_len = 0;
  struct zbus_ctx *recv = zbus_rxfinish();
  if (recv && recv->len) {
#ifdef ENC28J60_SUPPORT
    memcpy(uip_buf + ZBUS_BRIDGE_OFFSET, recv->data, recv->len);
    uip_len = recv->len;
#else
    memcpy(uip_buf, recv->data, recv->len);
    uip_len = recv->len;
    
    uip_input();
#endif
    /* reset the recieve buffer */
    recv->len = 0;
  }
  if (!uip_len)
    return;
  /* send buffer out */
  fill_llh_and_transmit ();

  uip_len = 0;
}
