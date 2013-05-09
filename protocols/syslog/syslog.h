/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef _SYSLOG_H
#define _SYSLOG_H

#include <avr/pgmspace.h>
#include "protocols/uip/uip.h"

#define MAX_DYNAMIC_SYSLOG_BUFFER 500

uint8_t syslog_send_P(PGM_P message);
uint8_t syslog_send(const char *message);
uint8_t syslog_sendf(const char *message, ...);
uint8_t syslog_sendf_P(PGM_P message, ...);
uint8_t syslog_send_ptr(void *message);

void syslog_flush (void);

/* Check the ARP/Neighbor cache for the necessary entries;
   return 0 if it's safe to send syslog data. */
uint8_t syslog_check_cache(void);

uip_ipaddr_t *syslog_getserver(void);

#endif /* _SYSLOG_H */
