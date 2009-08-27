/*
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
 */

#include "protocols/uip/uip.h"
#include "config.h"
#include "hardware/i2c/master/i2c_udp.h"
#include "i2c_udp_net.h"

void 
i2c_udp_net_init(void)
{
	uip_ipaddr_t ip;
	uip_ipaddr_copy(&ip, all_ones_addr);
	
	uip_udp_conn_t *i2c_conn = uip_udp_new(&ip, 0, i2c_udp_net_main);
	
	if(! i2c_conn) 
		return;					/* keine udp connection !? */
	
	uip_udp_bind(i2c_conn, HTONS(I2C_PORT));

	// Init the I2C Code
        i2c_udp_init(i2c_conn);
	
}

void
i2c_udp_net_main(void)
{
  if (uip_poll()) 
    i2c_udp_periodic();
  if (uip_newdata())
    i2c_udp_newdata();
}

/*
  -- Ethersex META --
  header(hardware/i2c/master/i2c_udp_net.h)
  net_init(i2c_udp_net_init)

  state_header(hardware/i2c/master/i2c_state.h)
  state_udp(struct i2c_connection_state_t i2c)
*/
