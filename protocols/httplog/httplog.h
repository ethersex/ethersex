/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

// httplog.h
//
// this is a literal copy of twitter.h with "twitter" replaced by "httplog"

#ifndef HAVE_HTTPLOG_H
#define HAVE_HTTPLOG_H

uint8_t httplog(char *);


#include "config.h"
#ifdef DEBUG_HTTPLOG
# include "core/debug.h"
# define HTTPLOG_DEBUG(a...)  debug_printf("httplog: " a)
#else
# define HTTPLOG_DEBUG(a...)
#endif

#endif  /* HAVE_HTTPLOG_H */
