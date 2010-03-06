/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2010 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef VFS_UTIL_H
#define VFS_UTIL_H

#include <stdint.h>

/* Copy file SRC to DEST.  Be aware, this function uses uip_buf
   as a temporary buffer. */
uint8_t vfs_copy_file (const char *dest, const char *src);

/* Opens a FILE, if it doesn't exist, it will be created. 
   returns a filehandle on success */
struct vfs_file_handle_t * vfs_open_or_creat(const char* filename);

/* Opens/creates a file and appends buffer to it */
uint8_t vfs_file_append(const char *filename, uint8_t* buf, uint16_t len);


#endif	/* VFS_UTIL_H */
