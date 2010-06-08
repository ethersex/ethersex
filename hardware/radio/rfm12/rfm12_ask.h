/**********************************************************
 * Copyright(C) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 *
 * @author      Gregor B.
 * @author      Dirk Pannenbecker
 * @date        22.01.2009

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

#ifndef __RFM12_ASK_H
#define __RFM12_ASK_H

enum ASK_TYPE {T_2272, T_1527, T_TEVION};
void rfm12_ask_tevion_send(uint8_t *, uint8_t *, uint8_t, uint8_t);
void rfm12_ask_2272_1527_switch(uint8_t);
void rfm12_ask_2272_send(uint8_t *, uint8_t, uint8_t);
void rfm12_ask_external_filter_init(void);
void rfm12_ask_external_filter_deinit(void);
void rfm12_ask_intertechno_send_bit(uint8_t);
void rfm12_ask_intertechno_send_sync();
void rfm12_ask_intertechno_send(uint8_t, uint8_t, uint8_t, uint8_t);
void rfm12_ask_sense_start(void);
void rfm12_ask_trigger(uint8_t, uint16_t);

#define INTERTECHNO_PERIOD 264 // produces pulse of 360 us

#endif /* __RFM12_ASK_H */
