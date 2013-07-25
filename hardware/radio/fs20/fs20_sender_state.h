/*
 *
 * Copyright (c) 2010 by Jens Wilmer <ethersex@jenswilmer.de>
 * Copyright (c) 2010 by Moritz Wenk <MoritzWenk@web.de>
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

#ifndef FS20_SENDER_STATE_H
#define FS20_SENDER_STATE_H

// State of coonection, new until acked or aborted, after that old
struct fs20_sender_connection_state_t {
  uint8_t state;
};

#endif /* FS20_SENDER_STATE_H */
