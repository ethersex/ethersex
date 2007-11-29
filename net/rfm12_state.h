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

#ifndef RFM12_STATE_H
#define RFM12_STATE_H

#include "../uip/psock.h"
#include "../pt/pt.h"
#include "../rfm12/rfm12.h"

struct rfm12_data {
  uint8_t toaddr;
  uint8_t fromaddr;
  uint8_t len;
  unsigned char data[0];
};

struct rfm12_connection_state_t {
  union {
    unsigned char inbuf[RFM12_DataLength];
    struct rfm12_data txdata;
  };
  uint8_t in_state;
  uint8_t in_len;
  uint8_t rfaddr;
  uint8_t out_len;
};

#endif
