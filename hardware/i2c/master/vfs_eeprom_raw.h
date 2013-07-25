
/*
 * Copyright (c) 2008 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef VFS_EEPROM_RAW_H
#define VFS_EEPROM_RAW_H

#include <stdlib.h>
#include "vfs_eeprom.h"

typedef struct {
  vfs_eeprom_inode_t inode; /* Points to the Block*/
  uint8_t end;

} vfs_file_handle_eeprom_raw_t;

/* vfs_eeprom_raw_ Prototypes. */
struct vfs_file_handle_t *vfs_eeprom_raw_open (const char *filename);
void vfs_eeprom_raw_close (struct vfs_file_handle_t *);
vfs_size_t vfs_eeprom_raw_read  (struct vfs_file_handle_t *, void *buf,
			         vfs_size_t length);

vfs_size_t vfs_eeprom_raw_write (struct vfs_file_handle_t *, void *buf,
			         vfs_size_t length);
/*
uint8_t vfs_df_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
		      uint8_t whence);
uint8_t vfs_df_truncate (struct vfs_file_handle_t *, vfs_size_t length);
struct vfs_file_handle_t *vfs_df_create (const char *name);
fs_size_t vfs_df_size (struct vfs_file_handle_t *); */


#define VFS_EEPROM_RAW_FUNCS {				\
    "ee_raw",	        				\
    vfs_eeprom_raw_open,				\
    vfs_eeprom_raw_close,				\
    vfs_eeprom_raw_read,				\
    vfs_eeprom_raw_write,				\
    NULL, /* fseek */                                   \
    NULL, /* truncate */                                \
    vfs_eeprom_raw_open, /* create */                   \
    NULL, /* unlink */                                  \
    NULL, /* filesize */                                \
  }

#endif	/* VFS_EEPROM_RAW_H */
