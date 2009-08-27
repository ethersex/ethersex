/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#ifndef _VNC_H
#define _VNC_H

#ifdef DEBUG_VNC
# include "core/debug.h"
# define VNCDEBUG(a...)  debug_printf("vnc: " a)
#else
# define VNCDEBUG(a...)
#endif

void vnc_init(void);
void vnc_periodic(void);
 
/*
* Copyright (c) 2001, Adam Dunkels.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote
*    products derived from this software without specific prior
*    written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This file is part of the uIP TCP/IP stack.
*
* $Id: vnc-server.h,v 1.1 2006/06/17 22:41:16 adamdunkels Exp $
*
*/

#include <inttypes.h>

/* From server to client: */
#define VNC_FB_UPDATE            0
#define VNC_SET_COLORMAP_ENTRIES 1
#define VNC_BELL                 2
#define VNC_SERVER_CUT_TEXT      3

/* From client to server. */
#define VNC_SET_PIXEL_FORMAT     0
#define VNC_FIX_COLORMAP_ENTRIES 1
#define VNC_SET_ENCODINGS        2
#define VNC_FB_UPDATE_REQ        3
#define VNC_KEY_EVENT            4
#define VNC_POINTER_EVENT        5
#define VNC_CLIENT_CUT_TEXT      6


#define RFB_BUTTON_MASK1 1
#define RFB_BUTTON_MASK2 2
#define RFB_BUTTON_MASK3 4
struct vnc_pointer_event {
  uint8_t type;
  uint8_t buttonmask;
  uint16_t x;
  uint16_t y;
};






#endif /* _VNC_H */
