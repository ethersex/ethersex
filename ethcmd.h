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

#ifndef _ETHCMD_H
#define _ETHCMD_H

#include <stdint.h>

#include "ethcmd/ethcmd_message.h"

struct ethcmd_connection_state_t {
    enum {
        ETHCMD_STATE_DISCONNECTED,
        ETHCMD_STATE_IDLE,
        ETHCMD_STATE_SEND_VERSION,
    } state;
};

#define ETHCMD_PORT 2847

void ethcmd_init(void);
void ethcmd_main(void);
void ethcmd_parse_message(struct ethcmd_message_t *msg);
void ethcmd_send(void);

#endif
