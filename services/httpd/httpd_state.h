/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _HTTPD_STATE_H
#define _HTTPD_STATE_H

#include "config.h"
#include "core/vfs/vfs.h"
#include "protocols/ecmd/via_tcp/ecmd_state.h"
#include "protocols/soap/soap.h"

typedef enum {
    HTTPD_STATE_CLOSED = 0,
    HTTPD_STATE_IDLE,
} http_state_t;

#define SD_DIR_MAX_DIRNAME_LEN 75

struct httpd_connection_state_t {
    unsigned header_acked		: 1;
    unsigned header_reparse		: 1;
    unsigned eof			: 1;

#ifdef HTTPD_AUTH_SUPPORT
    uint8_t auth_state;
#endif /* HTTP_AUTH_SUPPORT */

    /* The associated connection handler function */
    void (* handler)(void);

    union {
#ifdef VFS_SUPPORT
	struct {
	    /* The VFS file handle. */
	    struct vfs_file_handle_t *fd;

	    /* Content-type identifier char. */
	    unsigned char content_type;

	    vfs_size_t acked, sent;
	} vfs;
#endif	/* VFS_SUPPORT */

#ifdef HTTPD_SOAP_SUPPORT
	struct soap_context soap;
#endif	/* HTTPD_SOAP_SUPPORT */

#ifdef HTTP_SD_DIR_SUPPORT
	struct {
	    struct fat_dir_struct *handle;

	    union {
		struct fat_dir_entry_struct entries;
		char dirname[SD_DIR_MAX_DIRNAME_LEN];
	    };
	} dir;
#endif	/* HTTP_SD_DIR_SUPPORT */

#ifdef ECMD_PARSER_SUPPORT
	struct {
	    char input[ECMD_INPUTBUF_LENGTH];
	    char output[ECMD_OUTPUTBUF_LENGTH];
	} ecmd;
#endif	/* ECMD_PARSER_SUPPORT */

    } u;
};

#endif /* _HTTPD_STATE_H */
