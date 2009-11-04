/*
 * Copyright (c) 2009 by Sylwester Sosnowski <esc@ext.no-route.org>
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

#ifndef HAVE_USTREAM_H
#define HAVE_USTREAM_H

#define USTREAM_URI "GET /stream/1010 HTTP/1.0\r\n\r\n"

void ustream_init (void);
void ustream_periodic(void);

#include "config.h"
#ifdef DEBUG_USTREAM
# include "core/debug.h"
# define USTREAMDEBUG(a...)  debug_printf("ustream: " a)
#else
# define USTREAMDEBUG(a...)
#endif

#endif  /* HAVE_USTREAM_H */
