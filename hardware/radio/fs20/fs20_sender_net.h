/*
 * Copyright (c) 2010 by Moritz Wenk <MoritzWenk@web.de>
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


#ifndef _FS20_SENDER_NET_H
#define _FS20_SENDER_NET_H

// Convenient debugging
#include "config.h"

#ifdef DEBUG_FS20_SENDER
# include "core/debug.h"
# define FS20S_DEBUG(a...)  debug_printf("fs20sender: " a)
#else
# define FS20S_DEBUG(a...)
#endif

// Connectionstate: new until acked, aborted or timedout, old afterwards
enum Fs20ConnStates 
{
  FS20_CONNSTATE_NEW,
  FS20_CONNSTATE_OLD,
};

void fs20_sendmessage(void);
void fs20_sender_mainloop(void);

#endif /* _FS20_SENDER_NET_H */
