/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 }}} */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
        
#include "../config.h"
#include "../debug.h"
#include "i2c_master.h"
#include "i2c_24CXX.h"

#ifdef I2C_24CXX_SUPPORT
#define SFS_DEBUG_SUPPORT
#ifdef SFS_DEBUG_SUPPORT
#define sfs_debug(a...)  debug_printf("sfs: " a)
#else
#define sfs_debug(a...)  ()
#endif


#define sfs_read_page(page, data, len) i2c_24CXX_read_block(page * SFS_PAGE_SIZE, data, len)
#define sfs_read_slice(page, offset, data, len) i2c_24CXX_read_block(page * SFS_PAGE_SIZE + offset , data, len)
#define sfs_write_page(page, data, len) i2c_24CXX_write_block(page * SFS_PAGE_SIZE, data, len)
#define sfs_write_slice(page, offset, data, len) i2c_24CXX_write_block(page * SFS_PAGE_SIZE + offset , data, len)

void
sfs_init(void)
{
  unsigned char buf[SFS_PAGE_SIZE];
  sfs_read_page(0, buf, sizeof(struct sfs_page_superblock));

  /* Page 0  is the superblock */
  struct sfs_page_superblock *sb = (struct sfs_page_superblock *)buf;
  if (sb->magic != SFS_MAGIC_SUPERBLOCK || sb->identifier[0] != 0xdead 
      || sb->identifier[1] != 0xbeef || sb->version != SFS_VERSION) {
    sfs_debug("unformattet filesystem, format!\n");
    sfs_debug("magic: %x, ident %x%x, version: %x\n", sb->magic, 
              sb->identifier[0], sb->identifier[1], sb->version);
    sb->magic = SFS_MAGIC_SUPERBLOCK;
    sb->identifier[0] = 0xdead;
    sb->identifier[1] = 0xbeef;
    sb->version = SFS_VERSION;
    sb->next_page = 0;
    sb->next_file = 0;
    sfs_write_page(0, buf, sizeof(struct sfs_page_superblock));
    memset(buf, 0, 2);
    sfs_inode_t count = 1;
    while (count < 10)  {// FIXME SFS_PAGE_COUNT) {
      sfs_write_page(count, buf, 1);
      sfs_debug("clear page %d\n", count);
      count++;
    }
  } else {
    sfs_debug("detected, version %d\n", sb->version);
  }
  struct sfs_file_handle *file = sfs_open("index.html");
  if (!file)
    file = sfs_create("index.html");
//  file->offset = sfs_filesize(file);
  sfs_write(file, "Hallo Welt\n", 11); 
  sfs_debug("new_filesize ist: %d\n", sfs_filesize(file));
}

sfs_inode_t
sfs_find_free_page(sfs_inode_t suggested)
{
  unsigned char buf[1];

  sfs_inode_t tmp = suggested;

  while(1) {
    if (!sfs_read_page(tmp, buf, 1)) return 0;
    if (buf[0] != SFS_MAGIC_SUPERBLOCK && buf[0] != SFS_MAGIC_FILE && buf[0] != SFS_MAGIC_DATA) {
      sfs_debug("found empty page at %d\n", tmp);
      return tmp; /* yeah we have found a empty page */
    }
    tmp ++;
    if (tmp >= SFS_PAGE_COUNT) tmp = 0;
    if (tmp == suggested) return 0; /* 0 is the superblock, always so this indicates an error */
  }
}

static sfs_inode_t
sfs_find_file(char *filename)
{

  unsigned char buf[SFS_PAGE_SIZE];
  struct sfs_page_file *file = (struct sfs_page_file *) buf;
  struct sfs_page_superblock *sb = (struct sfs_page_superblock *) buf;
  
  if (!sfs_read_page(0, buf, sizeof(struct sfs_page_superblock))) return 0;
  sfs_inode_t inode = sb->next_file;

  while(inode) {
    sfs_read_page(inode, buf, SFS_PAGE_SIZE);
    if (file->magic != SFS_MAGIC_FILE) {
      sfs_debug("filesystem error: non file page is referenced\n");
      return 0;
    }
    if (filename && strcmp(file->filename, filename) == 0) {
      sfs_debug("file %s found at %d\n", filename, inode);
      return inode;
    }
    /* When the last file is searched */
    if (!filename && file->next_file == 0) {
      return inode;
    }
    inode = file->next_file;
  }
  sfs_debug("file %s not found\n", filename);
  return 0;
}

struct sfs_file_handle *
sfs_create(char * filename) 
{
  sfs_inode_t inode = sfs_find_file(filename);
  sfs_inode_t next_file = 0;

  unsigned char buf[SFS_PAGE_SIZE];
  struct sfs_page_file *file = (struct sfs_page_file *) buf;
  struct sfs_page_data *data = (struct sfs_page_data *) buf;

  if (inode == 0) {
    inode = sfs_find_free_page(inode);
    if (inode == 0) {
      sfs_debug("no space left on device\n");
      return NULL;
    }
    sfs_inode_t last_file = sfs_find_file(NULL); /* find the last file */
    sfs_debug("last file in chain is %d\n", last_file);
    sfs_write_slice(last_file, 3, (unsigned char *)&inode, 2);
  } else {
    sfs_read_page(inode, buf, SFS_PAGE_SIZE);
    /* save the next_file in the list */
    next_file = file->next_file;

    /* truncate the file */
    sfs_inode_t next_page = file->next_page;
    while(next_page) {
      sfs_read_page(next_page, buf, 4);
      sfs_inode_t tmp = data->next_page;
      memset(buf, 0, 4);
      sfs_write_page(next_page, buf, 4);
      sfs_debug("clear page %d\n", next_page);
      next_page = tmp;
    }
  }

  memset(buf, 0, SFS_PAGE_SIZE);
  file->magic = SFS_MAGIC_FILE;
  file->next_file = next_file,
  strncpy(file->filename, filename, sizeof(file->filename)); 
  sfs_write_page(inode, buf, SFS_PAGE_SIZE);

  struct sfs_file_handle *handle = malloc(sizeof(struct sfs_file_handle));
  if (!handle) return NULL;
  handle->file_page = inode;
  handle->offset = 0;

  return handle;
}

struct sfs_file_handle *
sfs_open(char * filename) 
{
  sfs_inode_t inode = sfs_find_file(filename);

  if (inode == 0) 
    return NULL;

  struct sfs_file_handle *handle = malloc(sizeof(struct sfs_file_handle));
  if (!handle) return NULL;
  handle->file_page = inode;
  handle->offset = 0;

  return handle;
}

void
sfs_close(struct sfs_file_handle *file)
{
  if (file)
    free(file);
}

sfs_ssize_t
sfs_filesize(struct sfs_file_handle *handle)
{
  unsigned char buf[SFS_PAGE_SIZE];
  struct sfs_page_file *file_page = (struct sfs_page_file *) buf;
  struct sfs_page_data *data_page = (struct sfs_page_data *) buf;

  if (!handle) return 0;
  sfs_read_page(handle->file_page, buf, SFS_PAGE_SIZE);
  /* count the already allocated pages */
  sfs_inode_t next_page = file_page->next_page;
  sfs_ssize_t size = 0;
  while(next_page) {
    sfs_debug("next_page: %d\n", next_page);
    sfs_read_page(next_page, buf, 4);
    size += data_page->page_len;
    if (data_page->page_len < sizeof(data_page->data))
      break;
    next_page = data_page->next_page;
  }
  return size;
}

uint8_t 
sfs_write(struct sfs_file_handle *handle, uint8_t *data, sfs_ssize_t len)
{  
  unsigned char buf[SFS_PAGE_SIZE];
  struct sfs_page_file *file_page = (struct sfs_page_file *) buf;
  struct sfs_page_data *data_page = (struct sfs_page_data *) buf;

  if (!handle) return 0;
  sfs_debug("write file %d, offset %d, len %d\n", handle->file_page, handle->offset, len);
  /* First we must determine if we need a new blocks at the end */
  sfs_inode_t pages_allocated = 0;

  sfs_read_page(handle->file_page, buf, SFS_PAGE_SIZE);
  /* count the already allocated pages */
  sfs_inode_t next_page = file_page->next_page;
  sfs_inode_t last_page = handle->file_page;
  sfs_inode_t write_page = 0;
  
  sfs_inode_t pages_skip = handle->offset / sizeof(data_page->data);
  sfs_debug("skip %d pages\n", pages_skip);

  while(next_page) {
    sfs_debug("next_page: %d\n", next_page);
    sfs_read_page(next_page, buf, 4);
    pages_allocated ++;
    last_page = next_page;


    if (!write_page) {
      /* Save here the last filled page */
      if (pages_skip == 0 || data_page->page_len < sizeof(data_page->data))
        write_page = next_page;

      if (pages_skip)
        pages_skip --;
    }
    next_page = data_page->next_page;
  }

  sfs_debug("write_block: %d (1)\n", write_page);

  /* our last page is full or the file page, we must use the first allocated page */
  if (last_page == handle->file_page)
    write_page = 0;

  sfs_inode_t pages_needed = (handle->offset + len) / sizeof(data_page->data) + 1;
  sfs_debug("pages allocated: %d, needed: %d, append %d pages to %d\n", 
            pages_allocated, pages_needed, pages_needed - pages_allocated,
            last_page);
  if (pages_needed > pages_allocated)
    pages_needed -= pages_allocated;
  else 
    pages_needed = 0;

  while(pages_needed--) {
    sfs_inode_t new_node = sfs_find_free_page(last_page + 1);
    if (new_node == 0) {
      sfs_debug("no space left on device\n");
      return 0;
    }
    /* Our last page was full, use the first allocated one */
    if (write_page == 0)
      write_page =  new_node;

    data_page->magic = SFS_MAGIC_DATA;
    data_page->next_page = 0;
    data_page->page_len = 0;
    sfs_write_page(new_node, buf, 4);
    sfs_write_slice(last_page, 1, (unsigned char *) &new_node, 2);
    last_page = new_node;
    sfs_debug("allocated new block at %d\n", new_node);
  }

  sfs_debug("first write page is %d\n", write_page);
  sfs_read_page(write_page, buf, SFS_PAGE_SIZE);
  sfs_inode_t write_offset = handle->offset % sizeof(data_page->data);
  handle->offset += len;

  while (len) {
    sfs_debug("page len %d %d\n", write_offset, data_page->page_len);
    sfs_size_t copy_len = len;
    if ((len + write_offset) > sizeof(data_page->data))
      copy_len = sizeof(data_page->data) - write_offset;

    sfs_debug("copy %d byte to page %d at %d\n", copy_len, write_page, write_offset);
    data_page->page_len = write_offset + copy_len;
    sfs_debug("%d, %d, %d, %d\n", write_page, 4 + write_offset, 
                    data_page->page_len, copy_len);

    sfs_write_slice(write_page, 4 + write_offset, 
                    data, copy_len);
    sfs_write_slice(write_page, 3, &data_page->page_len, 1);
    write_offset = 0;
    write_page = data_page->next_page;
    /* Read the next page */
    sfs_read_page(write_page, buf, SFS_PAGE_SIZE);
    data += copy_len;
    len -= copy_len;
  }

  return 0;
}

#endif /* I2C_24CXX_SUPPORT */


