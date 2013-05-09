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

#ifndef HAVE_JABBER_H
#define HAVE_JABBER_H

#define JABBER_VALUESIZE 16

char jabber_user[JABBER_VALUESIZE];
char jabber_pass[JABBER_VALUESIZE];
char jabber_resrc[JABBER_VALUESIZE];
char jabber_host[JABBER_VALUESIZE];

void jabber_init(void);
void jabber_periodic(void);
uint8_t jabber_send_message(char *message);

#ifdef DEBUG_JABBER
# include "core/debug.h"
# define JABDEBUG(a...)  debug_printf("jabber: " a)
#else
# define JABDEBUG(a...)
#endif

#endif  /* HAVE_JABBER_H */
