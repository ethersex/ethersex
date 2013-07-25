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


/**
* @defgroup dmx-storage
* @brief DMX Storage stores and manages DMX universes and provides routines for accessing the data.
*
* Additionally it provides a simple slot mechanism for a semi-push procedure using unique slots.
* Once connected with dmx_connect(), the client can poll for updates using his connection id (returned by dmx_connect())
* without traversing the whole universe.
*
* @code #include "service/dmx-storage/dmx_storage.h" @endcode
* @author Maximilian Güntner maximilian.guentner@gmail.com
*
*/
/*@{*/
enum dmx_state {DMX_UNCHANGED,DMX_NEWVALUES};
enum dmx_slot_used {DMX_SLOT_FREE,DMX_SLOT_USED};
struct dmx_slot
{
	enum dmx_state state;
	enum dmx_slot_used inuse;
};
/** 
 *  @name Functions
 */
/**
*	@brief Connects a module to an universe of dmx-storage
*	@param universe
*	@return connection id (>= 0) or -1 when all slots are full
*/
int8_t  dmx_storage_connect(uint8_t universe);
/**
*	@brief Disconnects a module from an universe of dmx-storage
*	@param universe
*	@param slot 
*	@return none
*/
void    dmx_storage_disconnect(uint8_t universe, int8_t slot);
/**
*	@brief Get a channel of an universe of dmx-storage
*	@param universe
*	@param channel
*	@return the channel value 
*/
uint8_t get_dmx_channel(uint8_t universe,uint16_t channel);
/**
*	@brief Returns a channel of an universe of dmx-storage using a slot
*
*	the same as get_dmx_channel but it will set the universe's state for the id to DMX_UNCHANGED
*	@param universe
*	@param channel
*	@param slot
*	@return the channel value 
*/
uint8_t get_dmx_channel_slot(uint8_t universe,uint16_t channel,int8_t slot);
/**
*	@brief Sets a channel of an universe of dmx-storage
*	@param universe
*	@param channel
*	@param value
*	@return 0 (success) or 1 (fail)
*/
uint8_t set_dmx_channel(uint8_t universe, uint16_t channel, uint8_t value);
/**
*	@brief Sets many channels of an universe of dmx-storage
*
*	After the channel has been set and the argument value is not equal to the old value
*	the state of the universe will be changed to DMX_NEWVALUES
*	@param *start Pointer to the head of DMX data
*	@param universe
*	@param len Length of the data
*	@return none
*/
void    set_dmx_channels(uint8_t *start, uint8_t universe,uint16_t len);
/**
*	@brief Gets the current state of an universe for a specific slot (connection id)
*	@param universe
*	@param slot
*	@return the state of the universe for the slot
*/
enum dmx_state get_dmx_universe_state(uint8_t universe,int8_t slot);
/*@}*/
