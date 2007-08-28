/* vim:fdm=marker ts=4 et ai
 * {{{
 *         simple rc5 implementation
 *
 *    for additional information please
 *    see http://lochraster.org/etherrape
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */


#ifndef RC5_H
#define RC5_H

/* configuration:
 *
 * if you want to use this library, define RC5_SUPPORT somewhere.
 * pin defines (eg for using PD4 for sending):
 *
 * #define RC5_SEND_PINNUM PD4
 * #define RC5_SEND_PORT PORTD
 * #define RC5_SEND_DDR DDRD
 *
 */

#include <stdint.h>

#ifdef RC5_SUPPORT

/* timing constants */

/* one pulse half is 889us, for _delay_loop_2 */
#define RC5_PULSE (F_CPU / 1000000 * 888 / 4)

/* prototypes */
void rc5_init(void);
void rc5_send(uint8_t addr, uint8_t cmd);

#endif
#endif
