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
#include "pt/pt.h"
#include "fs.h"

#define ETHCMD_BUFSIZE 20

struct ethcmd_connection_state_t {
    uint8_t timeout;
    union {
        char buffer[ETHCMD_BUFSIZE];
        union {
            struct ethcmd_msg_t raw;
            struct ethcmd_msg_version_t version;
            struct ethcmd_msg_fs20_t fs20;
            struct ethcmd_msg_storage_t storage;
            struct ethcmd_response_t response;
        } msg;
    };
    fs_index_t fs_index;
    fs_status_t fs_status;
    uint8_t filename[FS_FILENAME+1];
    uint8_t fill;
    uint16_t data_length;
    struct pt pt, datapt;
};

#define ETHCMD_PORT 2847
#define ETHCMD_TIMEOUT 100 /* in 200ms-steps */
#define ETHCMD_PROTOCOL_MAJOR 1
#define ETHCMD_PROTOCOL_MINOR 0

void ethcmd_init(void);
void ethcmd_main(void);

#endif
