/*
 * Copyright (c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef VFS_SD_H
#define VFS_SD_H

#include <stdlib.h>
#include "../dataflash/fs.h"
#include "../sd_reader/fat.h"

typedef struct fat_file_struct* vfs_file_handle_sd_t;

/* vfs_sd_ Prototypes. */
struct vfs_file_handle_t *vfs_sd_open (const char *filename);
void vfs_sd_close (struct vfs_file_handle_t *);
vfs_size_t vfs_sd_read  (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
vfs_size_t vfs_sd_write (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
uint8_t vfs_sd_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
		      uint8_t whence);
uint8_t vfs_sd_truncate (struct vfs_file_handle_t *, vfs_size_t length);
uint8_t vfs_sd_create (const char *name);
fs_size_t vfs_sd_size (struct vfs_file_handle_t *);


#define VFS_SD_FUNCS {				\
    "sd",					\
    vfs_sd_open,				\
    vfs_sd_close,				\
    vfs_sd_read,				\
    vfs_sd_write,				\
    vfs_sd_fseek,				\
    vfs_sd_truncate,				\
    vfs_sd_create,				\
    vfs_sd_size,				\
  }

#endif	/* VFS_SD_H */
