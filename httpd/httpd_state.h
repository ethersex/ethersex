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

#ifndef _HTTPD_STATE_H
#define _HTTPD_STATE_H

#include "../dataflash/fs.h"
#include "../config.h"
#include "../sd_reader/fat.h"

typedef enum {
    HTTPD_STATE_CLOSED = 0,
    HTTPD_STATE_IDLE,
} http_state_t;

struct httpd_connection_state_t {
    http_state_t state;
    uint8_t timeout;
    char buffer[40];
    char name[16];
    struct psock in, out;
    fs_inode_t inode;
    fs_size_t offset;

    char *tmp_buffer;
#if defined(SD_READER_SUPPORT)
    uint32_t len;
#elif defined(HTTPD_INLINE_FILES_SUPPORT)
    uint16_t len;
#endif
#ifdef ECMD_PARSER_SUPPORT
    uint8_t parse_again;
#endif
#ifdef SD_READER_SUPPORT
    struct fat_file_struct* fd;
#endif
};


#endif
