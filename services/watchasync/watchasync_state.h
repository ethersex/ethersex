/*
 *
 * Copyright (c) 2010 by Jens Wilmer <ethersex@jenswilmer.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#ifndef WATCHASYNC_STATE_H
#define WATCHASYNC_STATE_H

// Select Countersize
#ifndef WATCHASYNC_COUNTER_TYPE
#ifdef CONF_WATCHASYNC_32BITS
#define WATCHASYNC_COUNTER_TYPE uint32_t
#else // def CONF_WATCHASYNC_32BITS
#ifdef CONF_WATCHASYNC_16BITS
#define WATCHASYNC_COUNTER_TYPE uint16_t
#else // def CONF_WATCHASYNC_16BITS
#define WATCHASYNC_COUNTER_TYPE uint8_t
#endif // def CONF_WATCHASYNC_16BITS
#endif // def CONF_WATCHASYNC_32BITS
#endif // ndef WATCHASYNC_COUNTER_TYPE

// State of connection, new until acked or aborted, after that old
struct watchasync_connection_state_t {
  uint8_t state;
#ifdef CONF_WATCHASYNC_SUMMARIZE
  uint32_t timestamp;
  uint8_t pin;
  WATCHASYNC_COUNTER_TYPE count;
#endif
};

#endif /* WATCHASYNC_STATE_H */
