/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "protocols/uip/uip.h"
#include "core/debug.h"
#include "vnc.h"

#include "config.h"

uip_conn_t *vnc_conn = NULL;

static void 
vnc_main(void)
{
}


void vnc_init(void)
{
  uip_listen(HTONS(VNC_PORT), vnc_main);
}

/*
  -- Ethersex META --
  header(services/vnc/vnc.h)
  net_init(vnc_init)
*/
