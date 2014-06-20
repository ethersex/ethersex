/*
* Copyright (c) 2014 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdint.h>

#define ASK_TX_ENABLE
#define ASK_TX_DISABLE PIN_CLEAR(GENERIC_ASK_TX)
#define ASK_TX_TRIGGER generic_ask_trigger

void generic_ask_trigger(uint8_t, uint16_t);

#endif /* __GENERIC_H */
