/*
 * Copyright (c) 2021 by Frank Sautter <ethersix@sautter.com>
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

#include "config.h"

#ifndef ELTAKOMS_H
#define ELTAKOMS_H 1

#ifdef ELTAKOMS_SUPPORT

#ifdef DEBUG_ELTAKOMS
#include "core/debug.h"
#define ELTAKOMS_DEBUG(str...) debug_printf ("ELTAKOMS: " str)
#else
#define ELTAKOMS_DEBUG(...)    ((void) 0)
#endif

#include <stdint.h>

/* 
   example data:
   W+07.6016300N99901.2N?151515151515?1889

   0         1         2         3
   0123456789012345678901234567890123456789
   ||    | | | ||  |   ||| | | | | | ||   |
   ST    S S S OD  V   RWD M Y H M S DC   E
   TE    U U U BA  E   AEA O E O I E AH   T
   AM    N N N SW  L   IEY N A U N C YE   X
   RP          UN  O   NK  T R R U O LC
   TE    S W E R   C    D  H     T N IK
    R    O E A I   I    A        E D GS
    A    U S S T   T    Y            HU
    T    T T T Y   Y                 TM
    U    H                           S
    R                                A
    E                                V
                                     E
 */

#define ELTAKOMS_TEMP        1
#define ELTAKOMS_SUNSOUTH    6
#define ELTAKOMS_SUNWEST     8
#define ELTAKOMS_SUNEAST    10
#define ELTAKOMS_OBSCURITY  12
#define ELTAKOMS_DAWN       13
#define ELTAKOMS_WIND       16
#define ELTAKOMS_RAIN       20
#define ELTAKOMS_CHECKSUM   35

struct eltakoms_t
{
  int16_t  temperature;
  uint16_t wind;
  uint16_t dawn;
  uint8_t  suns;
  uint8_t  sunw;
  uint8_t  sune;
  uint8_t  valid:1;
  uint8_t  obscure:1;
  uint8_t  rain:1;
  uint8_t  dcf:1;
  uint8_t  :4;
  uint8_t  ptr;
  uint32_t ts;
  char  buffer[40];
};

extern struct eltakoms_t eltakoms_data;

void eltakoms_init(void);
void eltakoms_periodic(void);

#endif /* ELTAKOMS_SUPPORT */
#endif /* ELTAKOMS_H */
