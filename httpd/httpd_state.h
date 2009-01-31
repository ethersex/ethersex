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

#include "../config.h"
#include "../vfs/vfs.h"

typedef enum {
    HTTPD_STATE_CLOSED = 0,
    HTTPD_STATE_IDLE,
} http_state_t;

struct httpd_connection_state_t {
    unsigned header_acked		: 1;
    unsigned eof			: 1;

    /* The associated connection handler function */
    void (* handler)();

#ifdef VFS_SUPPORT
    /* The VFS file handle. */
    struct vfs_file_handle_t *fd;

    /* Content-type identifier char. */
    unsigned char content_type;

    vfs_size_t acked, sent;
#endif	/* VFS_SUPPORT */

#ifdef ECMD_PARSER_SUPPORT
    uint8_t parse_again;
#endif
};


#endif
