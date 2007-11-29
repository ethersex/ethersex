/**********************************************************
 Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/

#ifndef RFM12_NET_H
#define RFM12_NET_H

#include "rfm12_state.h"

// Port = (r + f + m) 12
#define RFM12_NET_PORT 32512 

extern union rx_buffer {
  unsigned char rxbuffer[RFM12_DataLength];
  struct rfm12_data rxdata;
} rx;



void rfm12_get_receive(void);
void rfm12_net_init(void);
void rfm12_net_main(void);

#endif
