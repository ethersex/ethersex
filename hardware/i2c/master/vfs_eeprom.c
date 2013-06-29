/*
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
 */

#include <avr/io.h>
#include <util/twi.h>
#include <string.h>

#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_master.h"
#include "hardware/i2c/master/i2c_24CXX.h"
#include "core/vfs/vfs.h"


#ifdef VFS_EEPROM_DEBUG_SUPPORT
#define vfs_eeprom_debug(a...)  debug_printf("vfs_eeprom: " a)
#else
#define vfs_eeprom_debug(a...)
#endif

#define vfs_eeprom_read_page(page, data, len) i2c_24CXX_read_block(page * SFS_PAGE_SIZE, data, len)
#define vfs_eeprom_read_slice(page, offset, data, len) i2c_24CXX_read_block(page * SFS_PAGE_SIZE + offset , data, len)
#define vfs_eeprom_write_page(page, data, len) i2c_24CXX_write_block(page * SFS_PAGE_SIZE, data, len)
#define vfs_eeprom_write_slice(page, offset, data, len) i2c_24CXX_write_block(page * SFS_PAGE_SIZE + offset , data, len)

void
vfs_eeprom_init(void)
{
  unsigned char buf[SFS_PAGE_SIZE];
  vfs_eeprom_read_page(0, buf, sizeof(struct vfs_eeprom_page_superblock));
  vfs_eeprom_debug("superblock %s\n", buf);

  /* Page 0 is the superblock */
  struct vfs_eeprom_page_superblock *sb =
    (struct vfs_eeprom_page_superblock *) buf;

  if (sb->magic != SFS_MAGIC_SUPERBLOCK ||
      sb->identifier[0] != 0xdead ||
      sb->identifier[1] != 0xbeef ||
      sb->version != SFS_VERSION)
  {
    vfs_eeprom_debug("unformatted filesystem, format!\n");
    vfs_eeprom_debug("magic: %x, ident %x%x, version: %x\n",
                     sb->magic,
                     sb->identifier[0], sb->identifier[1],
                     sb->version);

    sb->magic = SFS_MAGIC_SUPERBLOCK;
    sb->identifier[0] = 0xdead;
    sb->identifier[1] = 0xbeef;
    sb->version = SFS_VERSION;
    sb->next_page = 0;
    sb->next_file = 0;
    vfs_eeprom_write_page(0, buf, sizeof(struct vfs_eeprom_page_superblock));
    memset(buf, 0, 2);
    vfs_eeprom_inode_t count = 1;
    while (count < SFS_PAGE_COUNT)
    {
      wdt_kick();
      vfs_eeprom_write_page(count, buf, 1);
      vfs_eeprom_debug("clear page %d\n", count);
      count++;
    }
  }
  else
  {
    vfs_eeprom_debug("detected, version %d\n", sb->version);
  }
}

static vfs_eeprom_inode_t
vfs_eeprom_find_free_page(vfs_eeprom_inode_t suggested)
{
  unsigned char buf[1];

  vfs_eeprom_inode_t tmp = suggested;

  while (1)
  {
    if (!vfs_eeprom_read_page(tmp, buf, 1))
      return 0;
    if (buf[0] != SFS_MAGIC_SUPERBLOCK &&
        buf[0] != SFS_MAGIC_FILE &&
        buf[0] != SFS_MAGIC_DATA)
    {
      vfs_eeprom_debug("found empty page at %d\n", tmp);
      return tmp;               /* yeah we have found an empty page */
    }
    tmp++;
    if (tmp >= SFS_PAGE_COUNT)
      tmp = 0;
    if (tmp == suggested)
      return 0;                 /* 0 is the superblock, always so this indicates an error */
  }
}

static vfs_eeprom_inode_t
vfs_eeprom_find_file(const char *filename, vfs_eeprom_inode_t * prev_inode)
{
  unsigned char buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file = (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_superblock *sb =
    (struct vfs_eeprom_page_superblock *) buf;

  if (!vfs_eeprom_read_page(0, buf,
                            sizeof(struct vfs_eeprom_page_superblock)))
    return 0;

  if (prev_inode)
    *prev_inode = 0;
  vfs_eeprom_inode_t inode = sb->next_file;
  vfs_eeprom_debug("next file: %i\n", inode);

  while (inode)
  {
    vfs_eeprom_read_page(inode, buf, SFS_PAGE_SIZE);
    vfs_eeprom_debug("magic: %i ?=? %i\n", file->magic, SFS_MAGIC_FILE);
    vfs_eeprom_debug("file: %s \n", file->filename);
    if (file->magic != SFS_MAGIC_FILE)
    {
      vfs_eeprom_debug("filesystem error: non file page is referenced\n");
      return 0;
    }
    if (filename && strcmp(file->filename, filename) == 0)
    {
      vfs_eeprom_debug("file %s found at %d\n", filename, inode);
      return inode;
    }
    /* When the last file is searched */
    if (!filename && file->next_file == 0)
    {
      return inode;
    }
    if (prev_inode)
      *prev_inode = inode;
    inode = file->next_file;
  }
  vfs_eeprom_debug("file %s not found\n", filename);
  return 0;
}

struct vfs_file_handle_t *
vfs_eeprom_create(const char *filename)
{
  vfs_eeprom_inode_t inode = vfs_eeprom_find_file(filename, NULL);
  vfs_eeprom_inode_t next_file = 0;

  unsigned char buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file = (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_data *data = (struct vfs_eeprom_page_data *) buf;

  if (inode == 0)
  {
    inode = vfs_eeprom_find_free_page(inode);
    if (inode == 0)
    {
      vfs_eeprom_debug("no space left on device\n");
      return NULL;
    }
    vfs_eeprom_inode_t last_file = vfs_eeprom_find_file(NULL, NULL);    /* find the last file */
    vfs_eeprom_debug("last file in chain is %d\n", last_file);
    vfs_eeprom_write_slice(last_file, 3, (unsigned char *) &inode, 2);
  }
  else
  {
    vfs_eeprom_read_page(inode, buf, SFS_PAGE_SIZE);
    /* save the next_file in the list */
    next_file = file->next_file;

    /* truncate the file */
    vfs_eeprom_inode_t next_page = file->next_page;
    while (next_page)
    {
      vfs_eeprom_read_page(next_page, buf, 4);
      vfs_eeprom_inode_t tmp = data->next_page;
      memset(buf, 0, 4);
      vfs_eeprom_write_page(next_page, buf, 4);
      vfs_eeprom_debug("clear page %d\n", next_page);
      next_page = tmp;
    }
  }

  memset(buf, 0, SFS_PAGE_SIZE);
  file->magic = SFS_MAGIC_FILE;
  file->next_file = next_file,
    strncpy(file->filename, filename, sizeof(file->filename));
  vfs_eeprom_write_page(inode, buf, SFS_PAGE_SIZE);

  struct vfs_file_handle_t *handle = malloc(sizeof(struct vfs_file_handle_t));
  if (!handle)
    return NULL;
  handle->fh_type = VFS_EEPROM;
  handle->u.ee.file_page = inode;
  handle->u.ee.offset = 0;

  return handle;
}

struct vfs_file_handle_t *
vfs_eeprom_open(const char *filename)
{
  vfs_eeprom_inode_t inode = vfs_eeprom_find_file(filename, NULL);

  if (inode == 0)
    return NULL;

  struct vfs_file_handle_t *handle = malloc(sizeof(struct vfs_file_handle_t));
  if (!handle)
    return NULL;
  handle->fh_type = VFS_EEPROM;
  handle->u.ee.file_page = inode;
  handle->u.ee.offset = 0;

  return handle;
}

void
vfs_eeprom_close(struct vfs_file_handle_t *handle)
{
  if (handle)
    free(handle);
}

vfs_size_t
vfs_eeprom_filesize(struct vfs_file_handle_t *handle)
{
  unsigned char buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file_page =
    (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_data *data_page =
    (struct vfs_eeprom_page_data *) buf;

  if (!handle)
    return 0;
  vfs_eeprom_read_page(handle->u.ee.file_page, buf, SFS_PAGE_SIZE);
  /* count the already allocated pages */
  vfs_eeprom_inode_t next_page = file_page->next_page;
  vfs_size_t size = 0;
  while (next_page)
  {
    vfs_eeprom_debug("filesize; skip page: %d\n", next_page);
    vfs_eeprom_read_page(next_page, buf, 4);
    size += data_page->page_len;
    if (data_page->page_len < sizeof(data_page->data))
      break;
    next_page = data_page->next_page;
  }
  vfs_eeprom_debug("filesize; size: %d\n", size);
  return size;
}

vfs_size_t
vfs_eeprom_read(struct vfs_file_handle_t * handle, void *buffer,
                vfs_size_t size)
{
  uint8_t buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file_page =
    (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_data *data_page =
    (struct vfs_eeprom_page_data *) buf;
  vfs_size_t count = 0;

  if (!handle)
    return 0;
  vfs_eeprom_read_page(handle->u.ee.file_page, (uint8_t *) buf,
                       SFS_PAGE_SIZE);

  /* First we have to skip n pages to get to our offset */
  vfs_eeprom_inode_t pages_skip =
    handle->u.ee.offset / sizeof(data_page->data);
  vfs_eeprom_debug("read; skip %d pages\n", pages_skip);

  vfs_eeprom_inode_t next_page = file_page->next_page;

  while (pages_skip)
  {
    vfs_eeprom_debug("read; skip page: %d\n", next_page);
    vfs_eeprom_read_page(next_page, buf, 4);
    pages_skip--;
    next_page = data_page->next_page;
  }

  while (count < size)
  {
    vfs_eeprom_debug("read; read page: %d\n", next_page);
    vfs_eeprom_read_page(next_page, buf, SFS_PAGE_SIZE);
    if (data_page->magic != SFS_MAGIC_DATA)
      return 0;
    vfs_eeprom_len_t to_be_copied = sizeof(data_page->data);
    vfs_eeprom_len_t page_offset =
      handle->u.ee.offset % sizeof(data_page->data);
    uint8_t eof = 0;

    if ((size - count) < to_be_copied)
      to_be_copied = size - count;


    if (page_offset + to_be_copied > data_page->page_len)
    {
      vfs_eeprom_debug("read; offset: %d, to_be_copied: %d\n",
                       page_offset, to_be_copied);
      if (page_offset == 0)
        eof = 1;                /* We have reached the end of the file */
      to_be_copied = data_page->page_len - page_offset;
    }

    memcpy(buffer + count, data_page->data + page_offset, to_be_copied);

    count += to_be_copied;
    handle->u.ee.offset += to_be_copied;
    if (eof)
      break;
    next_page = data_page->next_page;
  }

  return count;
}

uint8_t
vfs_eeprom_fseek(struct vfs_file_handle_t * handle, vfs_size_t offset,
                 uint8_t whence)
{
  uint16_t new_pos;

  switch (whence)
  {
    case SEEK_SET:
      new_pos = offset;
      break;

    case SEEK_CUR:
      new_pos = handle->u.ee.offset + offset;
      break;

    case SEEK_END:
      new_pos = vfs_eeprom_filesize(handle) + offset;
      break;

    default:
      return -1;                /* Invalid argument. */
  }

  if (new_pos > vfs_eeprom_filesize(handle))
    return -1;                  /* Beyond end of file. */

  handle->u.ee.offset = new_pos;
  return 0;
}


vfs_size_t
vfs_eeprom_write(struct vfs_file_handle_t * handle, void *data,
                 vfs_size_t len)
{
  unsigned char buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file_page =
    (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_data *data_page =
    (struct vfs_eeprom_page_data *) buf;

  vfs_size_t written_len = len;

  if (!handle)
    return 0;
  vfs_eeprom_debug("write; file %d, offset %d, len %d\n",
                   handle->u.ee.file_page, handle->u.ee.offset, len);
  /* First we must determine if we need a new blocks at the end */
  vfs_eeprom_inode_t pages_allocated = 0;

  vfs_eeprom_read_page(handle->u.ee.file_page, buf, SFS_PAGE_SIZE);
  /* count the already allocated pages */
  vfs_eeprom_inode_t next_page = file_page->next_page;
  vfs_eeprom_inode_t last_page = handle->u.ee.file_page;
  vfs_eeprom_inode_t write_page = 0;

  vfs_eeprom_inode_t pages_skip =
    handle->u.ee.offset / sizeof(data_page->data);
  vfs_eeprom_debug("write; skip %d pages\n", pages_skip);

  while (next_page)
  {
    vfs_eeprom_debug("write; skip page: %d\n", next_page);
    vfs_eeprom_read_page(next_page, buf, 4);
    pages_allocated++;
    last_page = next_page;


    if (!write_page)
    {
      /* Save here the last filled page */
      if (pages_skip == 0 || data_page->page_len < sizeof(data_page->data))
        write_page = next_page;

      if (pages_skip)
        pages_skip--;
    }
    next_page = data_page->next_page;
  }

  /* our last page is full or the file page, we must use the first allocated page */
  if (last_page == handle->u.ee.file_page)
    write_page = 0;

  vfs_eeprom_inode_t pages_needed =
    (handle->u.ee.offset + len) / sizeof(data_page->data) + 1;
  vfs_eeprom_debug("pages allocated: %d, needed: %d, append %d pages to %d\n",
                   pages_allocated, pages_needed,
                   pages_needed - pages_allocated, last_page);
  if (pages_needed > pages_allocated)
    pages_needed -= pages_allocated;
  else
    pages_needed = 0;

  while (pages_needed--)
  {
    vfs_eeprom_inode_t new_node = vfs_eeprom_find_free_page(last_page + 1);
    if (new_node == 0)
    {
      vfs_eeprom_debug("no space left on device\n");
      return 0;
    }
    /* Our last page was full, use the first allocated one */
    if (write_page == 0)
      write_page = new_node;

    data_page->magic = SFS_MAGIC_DATA;
    data_page->next_page = 0;
    data_page->page_len = 0;
    vfs_eeprom_write_page(new_node, buf, 4);
    vfs_eeprom_write_slice(last_page, 1, (unsigned char *) &new_node, 2);
    last_page = new_node;
    vfs_eeprom_debug("write; alloc page %d\n", new_node);
  }

  vfs_eeprom_debug("write; first write page is %d\n", write_page);
  vfs_eeprom_read_page(write_page, buf, SFS_PAGE_SIZE);
  vfs_eeprom_inode_t write_offset =
    handle->u.ee.offset % sizeof(data_page->data);
  handle->u.ee.offset += len;

  while (len)
  {
    vfs_eeprom_len_t copy_len = len;
    if ((len + write_offset) > sizeof(data_page->data))
      copy_len = sizeof(data_page->data) - write_offset;

    vfs_eeprom_debug("write; copy %d byte to page %d at %d\n", copy_len,
                     write_page, write_offset);
    data_page->page_len = write_offset + copy_len;
    vfs_eeprom_debug("%d, %d, %d, %d\n", write_page, 4 + write_offset,
                     data_page->page_len, copy_len);

    vfs_eeprom_write_slice(write_page, 4 + write_offset, data, copy_len);
    vfs_eeprom_write_slice(write_page, 3, &data_page->page_len, 1);
    write_offset = 0;
    write_page = data_page->next_page;
    /* Read the next page */
    vfs_eeprom_read_page(write_page, buf, SFS_PAGE_SIZE);
    data += copy_len;
    len -= copy_len;
  }

  return written_len;
}

uint8_t
vfs_eeprom_unlink(const char *filename)
{
  uint8_t buf[SFS_PAGE_SIZE];
  struct vfs_eeprom_page_file *file_page =
    (struct vfs_eeprom_page_file *) buf;
  struct vfs_eeprom_page_data *data_page =
    (struct vfs_eeprom_page_data *) buf;

  vfs_eeprom_inode_t prev_inode;
  vfs_eeprom_inode_t inode = vfs_eeprom_find_file(filename, &prev_inode);

  if (inode == 0)
    return 1;                   /* file not found */

  vfs_eeprom_read_page(inode, buf, SFS_PAGE_SIZE);
  vfs_eeprom_inode_t next_page = file_page->next_page;
  vfs_eeprom_inode_t next_file = file_page->next_file;

  vfs_eeprom_read_page(prev_inode, buf, SFS_PAGE_SIZE);
  file_page->next_file = next_file;
  vfs_eeprom_write_page(prev_inode, buf, SFS_PAGE_SIZE);

  memset(buf, 0, SFS_PAGE_SIZE);
  vfs_eeprom_write_page(inode, buf, SFS_PAGE_SIZE);

  while (next_page)
  {
    vfs_eeprom_read_page(next_page, buf, 4);
    next_page = data_page->next_page;
    memset(buf, 0, SFS_PAGE_SIZE);
    vfs_eeprom_write_page(inode, buf, SFS_PAGE_SIZE);
  }

  return 0;
}

/*
  -- Ethersex META --
  header(hardware/i2c/master/vfs_eeprom.h)
  initearly(vfs_eeprom_init)
*/
