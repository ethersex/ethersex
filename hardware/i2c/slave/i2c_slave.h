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

//doesnt work
#ifndef CONF_TWI_SLAVE_ADDR
#define TWI_ADDR 0x04
#else
#define TWI_ADDR CONF_TWI_SLAVE_ADDR
#endif

//Status Codes for Slave Receiver Mode
#define TWI_SRX_ADR_ACK            0x60 //Own SLA+W has been received;ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80 //Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0 //A STOP condition or repeated START condition has been received while still addressed as Slave

//Status Codes for Slave Transmitter Mode
#define TWI_STX_ADR_ACK            0xA8 //Own SLA+R has been received;ACK has been returned
#define TWI_STX_DATA_ACK 		   0xB8 //Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK 		   0xC0 //Data byte in TWDR has been transmitted; NOT ACK has been received

//Miscellaneous States
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition

#ifdef DEBUG_TWI_SLAVE
# include "core/debug.h"
# define TWIDEBUG(a...)  debug_printf("twi sl: " a)
#else
# define TWIDEBUG(a...)
#endif


void twi_init (void);
unsigned char twi_busy( void );
int16_t twi_get_rx_data(char *cmd, int16_t len);
void twi_set_tx_data(char *cmd, int16_t len);
void parse_rawdata_twi_slave(void);
void twi_periodic(void);

#ifdef ECMD_TWI_SLAVE_SUPPORT
void ecmd_twi_periodic(void);
int16_t parse_ecmd_twi_slave (void);
#endif

#endif /* _TWI_SLAVE_H */
