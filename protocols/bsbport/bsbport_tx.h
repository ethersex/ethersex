/*
 *
 * Copyright (c) 2008 by Daniel Lindner <daniel.lindner@gmx.de>
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

#ifndef BSBPORT_TX_H
#define BSBPORT_TX_H

uint8_t bsbport_send(uint8_t * const msg);

uint8_t bsbport_query(const uint8_t A1, const uint8_t A2, const uint8_t A3,
                      const uint8_t A4, const uint8_t dest);
uint8_t bsbport_set(const uint8_t A1, const uint8_t A2, const uint8_t A3,
                    const uint8_t A4, const uint8_t dest,
                    const uint8_t * const data, const uint8_t datalen);

#endif /* BSBPORT_TX_H */
