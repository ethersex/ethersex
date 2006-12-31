/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 *          fs20 sender implementation
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


#ifndef _FS20_H
#define _FS20_H

#include "config.h"

#ifdef FS20_SUPPORT

#if !defined(FS20_PINNUM) || !defined(FS20_DDR) || !defined(FS20_PORT)
#error "FS20_PINNUM, FS20_DDR or FS20_PORT not defined!"
#endif

#if !defined(F_CPU)
#error "F_CPU undefined!"
#endif

#define FS20_DELAY_ZERO (4 * (F_CPU / 10000) / 4) /* 400uS, for delay_loop_2 */
#define FS20_DELAY_ONE  (6 * (F_CPU / 10000) / 4) /* 600uS, for delay_loop_2 */
#define FS20_DELAY_CMD  ( F_CPU / 10000 / 4) /* 10ms, for delay_loop_2 */

/* public prototypes */
void fs20_init(void);
void fs20_send(uint16_t housecode, uint8_t address, uint8_t command);

#endif

#endif
