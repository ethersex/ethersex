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
#define DEBUG_TWI

#include "config.h"
#ifdef DEBUG_TWI
# include "core/debug.h"
# define TWIDEBUG(a...)  debug_printf("twi: " a)
#else
# define TWIDEBUG(a...)
#endif

/* constants */
//#define I2C_SLAVE_PORT 0x2321

#ifndef CONF_I2C_SLAVE_ADDR
#define TWIADDR 0x04
#else
#define TWIADDR CONF_I2C_SLAVE_ADDR
#endif

#define ECMD_TWI_BUFFER_LEN 80

void twi_slave_init (void);
void twi_slave_periodic(void);
void twi_parse_ecmd (void);
//static uint8_t twi_parse_data (void);
//static void twi_handle_ecmd (void);


#endif /* _TWI_SLAVE_H */
