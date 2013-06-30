/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright(c) 2008 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include "df.h"

/* constants */
#define DF_PAGES 4096
#define DF_PAGESIZE 528

#define FS_INITIAL_VERSION 0x01
#define FS_STRUCTURE_OFFSET 0
#define FS_DATA_OFFSET 15
#define FS_CRC_OFFSET (DF_PAGESIZE-1)
#define FS_CRC_LENGTH (DF_PAGESIZE-FS_STRUCTURE_OFFSET-1)
#define FS_ROOTNODE_INODETABLE_SIZE 16
#define FS_ROOTNODE_INODETABLE_OFFSET (sizeof(fs_page_t)+sizeof(fs_version_t)) /* change this offset, if fs_root_t changed size! */
#define FS_ROOTNODE_NODETABLE_OFFSET (FS_ROOTNODE_INODETABLE_OFFSET + FS_ROOTNODE_INODETABLE_SIZE * sizeof(fs_inodetable_node_t))
#define FS_INODES_PER_TABLE 256 /* 512 databytes containing 2 bytes fs_inodetable_node_t structs */
#define FS_NODES_IN_ROOT ((DF_PAGESIZE-sizeof(fs_root_t)-1)/sizeof(fs_node_t))
#define FS_DATASIZE 512

#define FS_FILENAME 6

#define noinline __attribute__((noinline))

/* structs */
typedef uint32_t fs_version_t;
typedef uint16_t fs_inode_t;
typedef uint16_t fs_index_t;
typedef int32_t fs_size_t;

typedef enum {
    FS_OK = 0,
    FS_NOSUCHFILE = 1,
    FS_EOF = 2,
    FS_BADPAGE = 3,
    FS_BADINODE = 4,
    FS_DUPLICATE = 5,
    FS_MEM = 6,
    FS_BADSEEK = 7,
} fs_status_t;

/* generic filesystem structure */
typedef struct {
    df_chip_t chip;
    df_page_t root;
    fs_version_t version;
    df_page_t last_free;
} fs_t;

/* prototypes */

/* initialize filesystem, scan dataflash, format if no filesystem is found */
fs_status_t noinline fs_init(void);
/* list files in directory, write filename to buffer, return FS_OK or FS_EOF if no more */
fs_status_t noinline fs_list(fs_t *fs, char *dir, char *buf, fs_index_t index);
fs_inode_t noinline fs_get_inode(fs_t *fs, const char *file);
fs_size_t noinline fs_read(fs_t *fs, fs_inode_t inode, void *buf, fs_size_t offset, fs_size_t length);
fs_status_t noinline fs_write(fs_t *fs, fs_inode_t inode, void *buf, fs_size_t offset, fs_size_t length);
fs_status_t noinline fs_truncate(fs_t *fs, fs_inode_t inode, fs_size_t length);
fs_status_t noinline fs_create(fs_t *fs, const char *name);
fs_status_t noinline fs_remove(fs_t *fs, const char *name);
fs_size_t noinline fs_size(fs_t *fs, fs_inode_t inode);

/* local */
fs_status_t noinline fs_scan(fs_t *fs); /* scan for the root node */
fs_status_t noinline fs_format(fs_t *fs); /* format filesystem and create new root node in page 0 */
df_page_t noinline fs_new_page(fs_t *fs); /* return an empty (=unused) page or 0xffff if none could be found */
fs_inode_t noinline fs_new_inode(fs_t *fs); /* return an empty (=unused) inode or 0xffff if none could be found */
df_page_t noinline fs_inodetable(fs_t *fs, uint8_t tableid); /* return the page this inodetable lives in */
df_page_t noinline fs_page(fs_t *fs, fs_inode_t inode); /* get the page this inode points to */
void noinline fs_mark(fs_t *fs, df_page_t page, uint8_t free); /* mark page as used or free (cache in BUF2) */
#define fs_mark_free(fs, page) fs_mark(fs, page, 1)
#define fs_mark_used(fs, page) fs_mark(fs, page, 0)
uint8_t noinline fs_used(fs_t *fs, df_page_t page); /* check if this page is used */
uint8_t noinline fs_find(fs_t *fs, char *name); /* search for this name, return nodes[] index */
uint8_t noinline fs_crc(fs_t *fs, uint8_t crc, df_buf_t buf, df_size_t offset, df_size_t length); /* calculate crc in buffer */
fs_status_t noinline fs_increment(fs_t *fs); /* update version and crc of root node in BUF1, write BUF1 to a free page and update global pointer */
fs_status_t noinline fs_update_inodetable(fs_t *fs, fs_inode_t inode, df_page_t page); /* update inodetable and root node */

void fs_inspect_node(fs_t *fs, uint16_t p);
void fs_inspect_inode(fs_t *fs, uint16_t p);

extern fs_t fs;

#endif /* _FS_H */
