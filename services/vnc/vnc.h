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

/* Pixel format definition. */
struct vnc_pixel_format {
  uint8_t bps;       /* Bits per pixel: 8, 16 or 32. */
  uint8_t depth;     /* Color depth: 8-32 */
  uint8_t endian;    /* 1 - big endian (motorola), 0 - little endian
                       (x86) */
  uint8_t truecolor; /* 1 - true color is used, 0 - true color is not used. */

  /* The following fields are only used if true color is used. */
  uint16_t red_max, green_max, blue_max;
  uint8_t red_shift, green_shift, blue_shift;
  uint8_t pad1;
  uint16_t pad2;
};

struct vnc_server_init {
   uint16_t width;
   uint16_t height;
   struct vnc_pixel_format format;
   u8_t namelength[4];
   /* Followed by name. */
};




#endif /* _VNC_H */
