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

#ifndef VFS_DF_H
#define VFS_DF_H

#include <stdlib.h>
#include "hardware/storage/dataflash/fs.h"

typedef struct {
  fs_inode_t inode;
  fs_size_t offset;

} vfs_file_handle_df_t;

/* vfs_df_ Prototypes. */
struct vfs_file_handle_t *vfs_df_open (const char *filename);
void vfs_df_close (struct vfs_file_handle_t *);
vfs_size_t vfs_df_read  (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
vfs_size_t vfs_df_write (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
uint8_t vfs_df_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
		      uint8_t whence);
uint8_t vfs_df_truncate (struct vfs_file_handle_t *, vfs_size_t length);
struct vfs_file_handle_t *vfs_df_create (const char *name);
uint8_t vfs_df_unlink (const char *name);
vfs_size_t vfs_df_size (struct vfs_file_handle_t *);


#define VFS_DF_FUNCS {				\
    "df",					\
    vfs_df_open,				\
    vfs_df_close,				\
    vfs_df_read,				\
    vfs_df_write,				\
    vfs_df_fseek,				\
    vfs_df_truncate,				\
    vfs_df_create,				\
    vfs_df_unlink,				\
    vfs_df_size,				\
  }

#endif	/* VFS_DF_H */
