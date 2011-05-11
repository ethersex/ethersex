/*
 *
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#define DMX_NEWVALUES 1
#define DMX_UNCHANGED 0

uint8_t get_dmx_channel(uint16_t channel, uint8_t universe);
void set_dmx_channel(uint16_t channel, uint8_t universe,uint8_t value);
void set_dmx_channels(uint8_t *start, uint8_t universe,uint16_t len);
uint8_t get_dmx_universe_state(uint8_t universe);

