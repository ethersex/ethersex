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

#ifndef _I2C_STATE_H
#define _I2C_STATE_H

struct i2c_t {
	union{
		uint8_t raw[0];
		uint8_t seqnum;
	};
	union{
		uint8_t i2c_addr_rw;
		uint8_t maxdatalen;
		uint8_t write_datalen_ack;
		uint8_t readdata[0];
	};
	union{
		uint8_t datalen;
		uint8_t writedata[0];
	};
};

struct i2c_connection_state_t {
	uint8_t timeout;
	uint8_t last_seqnum;
};

#endif /* _I2C_STATE_H */
