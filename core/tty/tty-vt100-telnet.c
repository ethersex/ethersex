/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include "core/tty/tty.h"
#include "core/tty/tty-vt100-telnet.h"
#include "protocols/uip/uip.h"

static void
tty_vt100_main (void)
{
  if (uip_connected ())
    {
      /* Need to send complete off-screen image first. */
    }
}

void
tty_vt100_init (void)
{
  uip_listen(HTONS(TELNET_TCP_PORT), tty_vt100_main);
}

/*
  -- Ethersex META --
  header(core/tty/tty-vt100-telnet.h)
  net_init(tty_vt100_init)
*/
