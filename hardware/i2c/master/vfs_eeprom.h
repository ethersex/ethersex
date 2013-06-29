/*
 * Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
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

#ifndef VFS_EEPROM_H
#define VFS_EEPROM_H

#include "core/vfs/vfs.h"

/* We have to define this in Order to make the vfs_eeprom_page_file
 * compile. Be aware, that this is a WORKAROUND */
#ifndef SFS_PAGE_SIZE
	#define SFS_PAGE_SIZE 64
#endif
#ifndef SFS_PAGE_COUNT
	#define SFS_PAGE_COUNT 1
#endif

#define SFS_MAGIC_SUPERBLOCK 0x5
#define SFS_MAGIC_FILE 0x23
#define SFS_MAGIC_DATA 0x42

typedef uint16_t vfs_eeprom_inode_t;

#if SFS_PAGE_SIZE > 256
typedef uint16_t vfs_eeprom_len_t;
#else
typedef uint8_t vfs_eeprom_len_t;
#endif

#define SFS_VERSION 0


struct vfs_eeprom_page_superblock {
  uint8_t magic; /* The magic byte for a superblock is 0x5 */
  uint16_t next_page; /* not used within the superblock */
  uint16_t next_file; /* A Pointer to the next file page ( in pages ) */
  uint16_t identifier[2]; /* Should be 0xDEADBEEF */
  uint8_t version; /* Version of the Filesystem */
};

struct vfs_eeprom_page_file {
  uint8_t magic; /* The magic byte for a file is 0x23 */
  uint16_t next_page; /* A Pointer to the next data page ( in pages ) */
  uint16_t next_file; /* A Pointer to the next file page ( in pages ) */
  char filename[SFS_PAGE_SIZE - 6]; /* The Filename */
  char filename_0byte;
};

struct vfs_eeprom_page_data {
  uint8_t magic; /* The magic byte for a file is 0x42 */
  uint16_t next_page; /* A Pointer to the next data page ( in pages ) */
  uint8_t page_len; /* count of the used bytes in this page */
  char data[SFS_PAGE_SIZE - 4];
};

typedef struct {
  uint16_t file_page; /* the inode, were the file starts */ 
  uint16_t offset; /* the offset from the first  */
} vfs_file_handle_eeprom_t;


struct vfs_file_handle_t * vfs_eeprom_open(const char * filename);
void vfs_eeprom_init(void);
void vfs_eeprom_close(struct vfs_file_handle_t *handle);
vfs_size_t vfs_eeprom_write(struct vfs_file_handle_t *handle, void *buf, vfs_size_t len);
vfs_size_t vfs_eeprom_read(struct vfs_file_handle_t *handle, void *buffer, vfs_size_t size);
vfs_size_t vfs_eeprom_filesize(struct vfs_file_handle_t *handle);
uint8_t vfs_eeprom_fseek (struct vfs_file_handle_t *handle, vfs_size_t offset, uint8_t whence);
struct vfs_file_handle_t * vfs_eeprom_create(const char * filename);
uint8_t vfs_eeprom_unlink(const char * filename);


#define VFS_EEPROM_FUNCS {		\
    "ee",	        		\
    vfs_eeprom_open,			\
    vfs_eeprom_close,			\
    vfs_eeprom_read,			\
    vfs_eeprom_write,	                \
    vfs_eeprom_fseek,                   \
    NULL,      /* truncate */           \
    vfs_eeprom_create,                  \
    vfs_eeprom_unlink,                  \
    vfs_eeprom_filesize                 \
  }

#endif	/* VFS_EEPROM_H */
