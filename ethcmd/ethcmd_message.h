/* vim:fdm=marker ts=4 et ai
 * {{{
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

#ifndef _ETHCMD_MESSAGE_H
#define _ETHCMD_MESSAGE_H

#include <stdint.h>

#define ETHCMD_MESSAGE_TYPE_VERSION 0x0000
#define ETHCMD_MESSAGE_TYPE_ONEWIRE 0x0005

struct ethcmd_message_t {
    uint16_t length;
    uint16_t message_type;
    uint8_t data[];
};

struct ethcmd_onewire_message_t {
    uint8_t id[8];
};

#endif
