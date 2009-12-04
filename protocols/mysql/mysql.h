/*
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

#ifndef HAVE_MYSQL_H
#define HAVE_MYSQL_H

#define MYSQL_SERVER_PROTO_VERSION 10
#include <stdint.h>

void mysql_init (void);
void mysql_periodic (void);
uint8_t mysql_send_message (char *message);

#ifdef DEBUG_MYSQL
# include "core/debug.h"
# define MYDEBUG(a...)  debug_printf("mysql: " a)
#else
# define MYDEBUG(a...)
#endif

#endif  /* HAVE_MYSQL_H */
