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

#ifndef VFS_INLINE_H
#define VFS_INLINE_H

#include <stdlib.h>

#define VFS_INLINE_MAGIC 0x23
#define VFS_INLINE_FNLEN 6

union vfs_inline_node_t {
  struct __attribute__((__packed__)) {
    char fn[VFS_INLINE_FNLEN];
    uint16_t len;
    uint8_t crc;
  } s ;

  unsigned char raw[0];
};

typedef struct {
  vfs_size_t offset;		/* Offset in program memory. */
  uint16_t pos;			/* Position in file. */
  uint16_t len;			/* Length of file. */
} vfs_file_handle_inline_t;

/* vfs_sd_ Prototypes. */
struct vfs_file_handle_t *vfs_inline_open (const char *filename);
void vfs_inline_close (struct vfs_file_handle_t *);
vfs_size_t vfs_inline_read  (struct vfs_file_handle_t *, void *buf,
			 vfs_size_t length);
vfs_size_t vfs_inline_size (struct vfs_file_handle_t *);
uint8_t vfs_inline_fseek (struct vfs_file_handle_t *, vfs_size_t offset,
			  uint8_t whence);


#define VFS_INLINE_FUNCS {		\
    "inline",				\
    vfs_inline_open,			\
    vfs_inline_close,			\
    vfs_inline_read,			\
    NULL, /* write */			\
    vfs_inline_fseek,			\
    NULL, /* truncate */		\
    NULL, /* create */			\
    NULL, /* unlink */			\
    vfs_inline_size,			\
  }

#endif	/* VFS_INLINE_H */
