/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef VFS_DC3840_H
#define VFS_DC3840_H

#include <stdlib.h>

typedef struct {
  uint16_t pos;			/* Position in file. */
} vfs_file_handle_dc3840_t;

/* vfs_sd_ Prototypes. */
struct vfs_file_handle_t *vfs_dc3840_open (const char *filename);
void vfs_dc3840_close (struct vfs_file_handle_t *);
vfs_size_t vfs_dc3840_read  (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
vfs_size_t vfs_dc3840_size (struct vfs_file_handle_t *);
uint8_t vfs_dc3840_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
			  uint8_t whence);


#define VFS_DC3840_FUNCS {		\
    "dc3840",				\
    vfs_dc3840_open,			\
    vfs_dc3840_close,			\
    vfs_dc3840_read,			\
    NULL, /* write */			\
    NULL, /* fseek */			\
    NULL, /* truncate */		\
    NULL, /* create */			\
    NULL, /* unlink */			\
    NULL, /* size */			\
  }

#endif	/* VFS_DC3840_H */
