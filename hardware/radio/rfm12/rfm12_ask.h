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

void rfm12_ask_tevion_send(uint8_t *, uint8_t *, uint8_t, uint8_t);
void rfm12_ask_2272_send(uint8_t *, uint8_t, uint8_t);
void rfm12_ask_external_filter_init(void);
void rfm12_ask_external_filter_deinit(void);

#endif /* __RFM12_ASK_H */
