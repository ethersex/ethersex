/*
 * Copyright (c) 2009 by Bernd Stellwag <burned@zerties.org>
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef _TWI_SLAVE_H
#define _TWI_SLAVE_H

//Status Codes for Slave Receiver Mode
#define TWI_SRX_ADR_ACK            0x60 //Own SLA+W has been received;ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0 //A STOP condition or repeated START condition has been received while still addressed as Slave

//Status Codes for Slave Transmitter Mode
#define TWI_STX_ADR_ACK            0xA8 //Own SLA+R has been received;ACK has been returned
#define TWI_STX_DATA_ACK 		   0xB8 //Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK 		   0xC0 //Data byte in TWDR has been transmitted; NOT ACK has been received

#ifdef DEBUG_I2C_SLAVE
# include "core/debug.h"
# define TWIDEBUG(a...)  debug_printf("twi: " a)
#else
# define TWIDEBUG(a...)
#endif
/*
#ifndef CONF_I2C_SLAVE_ADDR
#define TWI_ADDR 0x04
#else
#define TWI_ADDR CONF_I2C_SLAVE_ADDR
#endif
*/
//test
#define TWI_ADDR 0x04

#define ECMD_I2C_SLAVE_SUPPORT TRUE

#define TWI_BUFFER_LEN 50

void twi_init (void);
void twi_periodic(void);

//#ifdef ECMD_I2C_SLAVE_SUPPORT
int16_t parse_cmd_twi_slave (char *cmd, char *output, uint16_t len);
//#endif

#endif /* _TWI_SLAVE_H */
