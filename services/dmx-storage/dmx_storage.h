/*
 *
 * Copyright (c) 2011-2015 by Maximilian Güntner <maximilian.guentner@gmail.com>
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

#ifndef _DMX_STORAGE_H
#define _DMX_STORAGE_H


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
enum dmx_slot_state
{ DMX_UNCHANGED, DMX_NEWVALUES };
enum dmx_universe_state
{ DMX_LIVE, DMX_BLACKOUT };
enum dmx_slot_used
{ DMX_SLOT_FREE, DMX_SLOT_USED };

struct dmx_slot
{
  enum dmx_slot_state slot_state;
  enum dmx_slot_used inuse;
};

struct dmx_universe
{
  uint8_t channels[DMX_STORAGE_CHANNELS];
  struct dmx_slot slots[DMX_STORAGE_SLOTS];
  enum dmx_universe_state universe_state;
  uint8_t dimmer;
};

/**
 *  @name Functions
 */
/**
*	@brief Connects a module to a universe of dmx-storage
*	@param universe
*	@return connection id (>= 0) or -1 when all slots are full
*/
int8_t dmx_storage_connect(uint8_t universe);
/**
*	@brief Disconnects a module from a universe of dmx-storage
*	@param universe
*	@param slot
*	@return none
*/
void dmx_storage_disconnect(uint8_t universe, int8_t slot);
/**
*	@brief Get a channel of a universe
*	@param universe
*	@param channel
*	@return the channel value
*/
uint8_t get_dmx_channel(uint8_t universe, uint16_t channel);
/**
*	@brief Gets the raw value of a channel of a universe
*	@param universe
*	@param channel
*	@return the channel value
*/
uint8_t get_dmx_channel_raw(uint8_t universe, uint16_t channel);
/**
*	@brief Returns a channel of a universe of dmx-storage using a slot
*
*	the same as get_dmx_channel but it will set the universe's state for the id to DMX_UNCHANGED
*	@param universe
*	@param channel
*	@param slot
*	@return the channel value
*/
uint8_t get_dmx_channel_slot(uint8_t universe, uint16_t channel, int8_t slot);
/**
*	@brief Returns a channel of a universe of dmx-storage using a slot
*
*	the same as get_dmx_channel_slot without applying a dimmer
*	@param universe
*	@param channel
*	@param slot
*	@return the channel value
*/
uint8_t get_dmx_channel_slot_raw(uint8_t universe, uint16_t channel, int8_t slot);
/**
*	@brief Sets a channel of a universe of dmx-storage
*	@param universe
*	@param channel
*	@param value
*	@return 0 (success) or 1 (fail)
*/
uint8_t set_dmx_channel(uint8_t universe, uint16_t channel, uint8_t value);
/**
*	@brief Sets many channels of a universe of dmx-storage
*
*	After the channel has been set and the argument value is not equal to the old value
*	the state of the universe will be changed to DMX_NEWVALUES
*	@param *channel_data Pointer to the head of DMX data
*	@param universe
*	@param start_from_channel Index of first channel to be changed
*	@param len Length of the data
*	@return none
*/
void set_dmx_channels(const uint8_t * channel_data, uint8_t universe, uint16_t start_from_channel, uint16_t len);
/**
*	@brief Gets the current state of a universe for a specific slot (connection id)
*	@param universe
*	@param slot
*	@return the state of the universe for the slot
*/
enum dmx_slot_state get_dmx_slot_state(uint8_t universe, int8_t slot);

/**
*	@brief Gets the current state of a universe
*	@param universe
*	@return the state of the universe
*/
enum dmx_universe_state get_dmx_universe_state(uint8_t universe);

/**
*	@brief Sets the state of a universe
*	@param universe
*	@param state
*	@return none
*/
void set_dmx_universe_state(uint8_t universe, enum dmx_universe_state state);

/**
 * @brief returns the dimmer of universe
 * @param universe
 * @return the current dimmer setting
 */
uint8_t get_dmx_universe_dimmer(uint8_t universe);

/**
 * @brief sets the dimmer value for universe
 * @param universe
 * @return none
 */
void set_dmx_universe_dimmer(uint8_t universe, uint8_t value);

/**
*	@brief Initializes the dmx storage module
*	@param none
*	@return none
*/
void dmx_storage_init(void);

/*@}*/

#endif /* end of include guard: _DMX_STORAGE_H */
