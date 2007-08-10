/*
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#ifndef _TFTP_H
#define _TFTP_H

#include <stdint.h>

/* tftp packet header */
struct tftp_hdr {
    uint16_t type;
    
    union {
	char raw[0];
	
	struct {
	    uint16_t block;
	    char data[0];
	} data;

	struct {
	    uint16_t block;
	} ack;

	struct {
	    uint16_t code;
	    char msg[1];
	} error;
    } u;
};


/* prototypes */
void tftp_handle_packet(void);

#endif
