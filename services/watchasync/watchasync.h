/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 * Copyright (c) 2010 by Jens Wilmer <ethersex@jenswilmer.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _WATCHASYNC_H
#define _WATCHASYNC_H

// Convenient debugging
#include "config.h"
#ifdef DEBUG_WATCHASYNC
# include "core/debug.h"
# define WATCHASYNC_DEBUG(a...)  debug_printf("watchasync: " a)
#else
# define WATCHASYNC_DEBUG(a...)
#endif

// Connectionstate: new until acked, aborted or timedout, old afterwards
enum WatchAsyncConnStates {
  WATCHASYNC_CONNSTATE_NEW,
  WATCHASYNC_CONNSTATE_OLD,
};

// Buffer for storing events of interrupt-routine
struct WatchAsyncBuffer {
  uint8_t pin;
#ifdef CONF_WATCHASYNC_INCLUDE_TIMESTAMP
  uint32_t timestamp;
#endif
};

void watchasync_init();
void watchasync_mainloop();

#ifdef CONF_WATCHASYNC_EDGDETECTVIAPOLLING
void watchasync_periodic(void);
#else
#define watchasync_periodic() do {} while(0)
#endif

#endif /* _WATCHASYNC_H */
