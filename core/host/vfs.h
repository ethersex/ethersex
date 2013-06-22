/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef CORE_HOST_VFS_H
#define CORE_HOST_VFS_H

#include <stdint.h>
#include "core/vfs/vfs.h"

typedef struct {
  int fd;
} vfs_file_handle_host_t;

/* vfs_sd_ Prototypes. */
struct vfs_file_handle_t *vfs_host_open (const char *filename);
void vfs_host_close (struct vfs_file_handle_t *);
vfs_size_t vfs_host_read  (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
vfs_size_t vfs_host_size (struct vfs_file_handle_t *);
uint8_t vfs_host_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
			  uint8_t whence);


#define VFS_HOST_FUNCS {		\
    "host",				\
    vfs_host_open,			\
    vfs_host_close,			\
    vfs_host_read,			\
    NULL, /* write */			\
    vfs_host_fseek,			\
    NULL, /* truncate */		\
    NULL, /* create */			\
    NULL, /* unlink */			\
    NULL, /* size */			\
  }

#endif  /* CORE_HOST_VFS_H */
