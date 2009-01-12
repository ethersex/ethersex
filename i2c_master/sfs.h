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

typedef uint16_t sfs_inode_t;
typedef uint16_t sfs_offset_t;
typedef uint16_t sfs_ssize_t;

#define SFS_PAGE_SIZE 128
#define SFS_PAGE_COUNT 512

#define SFS_MAGIC_SUPERBLOCK 0x5
#define SFS_MAGIC_FILE 0x23
#define SFS_MAGIC_DATA 0x42

#if SFS_PAGE_SIZE > 256
typedef uint16_t sfs_size_t;
#else
typedef uint8_t sfs_size_t;
#endif

#define SFS_VERSION 0

struct sfs_file_handle * sfs_create(char * filename);
struct sfs_file_handle * sfs_open(char * filename);
void sfs_close(struct sfs_file_handle *file);
uint8_t sfs_write(struct sfs_file_handle *file, unsigned char *buf, sfs_ssize_t len);
sfs_ssize_t sfs_filesize(struct sfs_file_handle *handle);

struct sfs_page_superblock {
  uint8_t magic; /* The magic byte for a file is 0x23 */
  uint16_t next_page; /* not used within the superblock */
  uint16_t next_file; /* A Pointer to the next file page ( in pages ) */
  uint16_t identifier[2]; /* Should be 0xDEADBEEF */
  uint8_t version; /* Version of the Filesystem */
};

struct sfs_page_file {
  uint8_t magic; /* The magic byte for a file is 0x23 */
  uint16_t next_page; /* A Pointer to the next data page ( in pages ) */
  uint16_t next_file; /* A Pointer to the next file page ( in pages ) */
  char filename[SFS_PAGE_SIZE - 6]; /* The Filename */
  char filename_0byte;
};

struct sfs_page_data {
  uint8_t magic; /* The magic byte for a file is 0x42 */
  uint16_t next_page; /* A Pointer to the next data page ( in pages ) */
  uint8_t page_len; /* count of the used bytes in this page */
  char data[SFS_PAGE_SIZE - 4];
};

struct sfs_file_handle {
  uint16_t file_page; /* the inode, were the file starts */ 
  uint16_t offset; /* the offset from the first  */
};
