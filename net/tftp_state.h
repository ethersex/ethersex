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

#ifndef TFTP_STATE_H
#define TFTP_STATE_H

#include "tftp_net.h"

/* state */
struct tftp_connection_state_t {
    unsigned       download    :1;
    unsigned       finished    :1;
    unsigned       bootp_image :1;
    unsigned       fire_req    :1;		/* this connection is for just
						 * starting a tftp request */

    uint16_t       transfered;			/* also retry countdown */
    unsigned char  filename[TFTP_FILENAME_MAXLEN];
};

#endif /* TFTP_STATE_H */
