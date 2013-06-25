/*
 * Copyright (c) 2007 by Jochen Roessner <jochen@lugrot.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _I2C_SLAVE_H
#define _I2C_SLAVE_H

#include <stdint.h>
#include "protocols/uip/uip.h"

/* constants */
#define I2C_SLAVE_PORT 0x2321
#ifndef CONF_I2C_SLAVE_ADDR
#define TWIADDR 0x04
#else
#define TWIADDR CONF_I2C_SLAVE_ADDR
#endif

/* prototypes */
void i2c_slave_core_init(uip_udp_conn_t * i2c_slave_conn);
void i2c_slave_core_periodic(void);
void i2c_slave_core_newdata(void);

#endif /* _I2C_SLAVE_H */
