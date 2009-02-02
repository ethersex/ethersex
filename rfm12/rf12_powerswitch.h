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

#ifndef __RF12_POWERSWITCH_H
#define __RF12_POWERSWITCH_H

void rf12_powerswitch_tevion_send(uint8_t *, uint8_t *, uint8_t, uint8_t);
void rf12_powerswitch_22772_send(uint8_t *, uint8_t, uint8_t);
void rf12_powerswitch_receiver_init(void);
void rf12_powerswitch_receiver_deinit(void);

#endif //__RF12_POWERSWITCH_H
