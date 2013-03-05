/*
 * Copyright (c) Gregor B.
 * Copyright (c) 2009 Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2012-13 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __RFM12_ASK_H
#define __RFM12_ASK_H

void rfm12_ask_tevion_send(uint8_t *, uint8_t *, uint8_t, uint8_t);
void rfm12_ask_2272_send(uint8_t *, uint8_t, uint8_t);
void rfm12_ask_1527_send(uint8_t *, uint8_t, uint8_t);
void rfm12_ask_intertechno_send(uint8_t, uint8_t, uint8_t, uint8_t);
void rfm12_ask_oase_send(uint8_t *, uint8_t, uint8_t);
void rfm12_ask_trigger(uint8_t, uint16_t);
void rfm12_ask_external_filter_init(void);
void rfm12_ask_external_filter_deinit(void);
void rfm12_ask_init(void);

#endif /* __RFM12_ASK_H */
