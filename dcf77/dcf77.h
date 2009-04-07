/*
 *
 * Copyright (c) 2007 by Christian Dietrich <stettberger@dokucode.de>
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
#ifndef _DCF77_H
#define _DCF77_H

#include "config.h"


struct dcf77_ctx {
  /* dcf_time: dummy, flags (S,A2,Z2,Z1,A1,R,x,x), min, stunde, tag, wochentag, monat, jahr */
  uint8_t time[0x8];
  uint32_t timerover;
  uint8_t TCNT2last;
  uint8_t timebyte;
  uint8_t timeparity;
  uint8_t sync;
  uint8_t valid;
};

void dcf77_init(void);

#endif
