/*
 * Copyright (c) 2008 by Jochen Roessner <jochen@lugrot.de>
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
#include "hardware/i2c/slave/i2c_slave.h"
#include "i2c_slave_net.h"

void
i2c_slave_net_init(void)
{
  uip_ipaddr_t ip;
  uip_ipaddr_copy(&ip, all_ones_addr);

  uip_udp_conn_t *i2c_slave_conn = uip_udp_new(&ip, 0, i2c_slave_net_main);

  if (!i2c_slave_conn)
    return;                     /* keine udp connection !? */

  uip_udp_bind(i2c_slave_conn, HTONS(I2C_SLAVE_PORT));

  // Init the I2C Code
  i2c_slave_core_init(i2c_slave_conn);

}

void
i2c_slave_net_main(void)
{
  if (uip_poll())
    i2c_slave_core_periodic();
  if (uip_newdata())
    i2c_slave_core_newdata();
}

/*
  -- Ethersex META --
  header(hardware/i2c/slave/i2c_slave_net.h)
  net_init(i2c_slave_net_init)

  state_header(hardware/i2c/slave/i2c_slave_state.h)
  state_udp(struct i2c_slave_connection_state_t i2c_slave)
*/
