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

#include <avr/pgmspace.h>

#include <stdlib.h>

#include "core/eeprom.h"
#include "core/vfs/vfs.h"

#if FLASHEND > UINT16_MAX
#define __pgm_read_byte pgm_read_byte_far
#else
#define __pgm_read_byte pgm_read_byte_near
#endif


struct vfs_file_handle_t *
vfs_inline_open (const char *filename)
{
  vfs_size_t offset = FLASHEND - SPM_PAGESIZE + 1;
  for (; offset; offset -= SPM_PAGESIZE) {
    if (__pgm_read_byte (offset) != VFS_INLINE_MAGIC)
      continue;

    union vfs_inline_node_t node;
    uint8_t i;

    for (i = 0; i < sizeof (node); i ++)
      node.raw[i] = __pgm_read_byte (offset + i + 1);

    if (node.s.crc != crc_checksum (node.raw, sizeof (node) - 1))
      continue;

    if (strncmp (node.s.fn, filename, VFS_INLINE_FNLEN))
      continue;

    /* Found file, create a handle. */
    struct vfs_file_handle_t *fh = malloc (sizeof (struct vfs_file_handle_t));
    if (fh == NULL)
      return NULL;

    fh->fh_type = VFS_INLINE;
    fh->u.il.offset = offset + sizeof (union vfs_inline_node_t) + 1;
    fh->u.il.pos = 0;
    fh->u.il.len = node.s.len;
    return fh;
  }

  return NULL;			/* File not found. */
}

#ifndef VFS_TEENSY
void
vfs_inline_close (struct vfs_file_handle_t *fh)
{
  free (fh);
}
#endif	/* VFS_TEENSY */

vfs_size_t
vfs_inline_read (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  vfs_size_t len = fh->u.il.len - fh->u.il.pos;
  if (length < len) len = length;

  for (vfs_size_t i = 0; i < len; i ++)
    ((unsigned char *)buf)[i] =
      __pgm_read_byte (fh->u.il.offset + fh->u.il.pos + i);

  fh->u.il.pos += (uint16_t)len;
  return len;
}

#ifndef VFS_TEENSY
uint8_t
vfs_inline_fseek (struct vfs_file_handle_t *fh, vfs_size_t offset,
		  uint8_t whence)
{
  vfs_size_t new_pos;

  switch (whence)
    {
    case SEEK_SET:
      new_pos = offset;
      break;

    case SEEK_CUR:
      new_pos = fh->u.il.pos + offset;
      break;

    case SEEK_END:
      new_pos = fh->u.il.len + offset;
      break;

    default:
      return -1;		/* Invalid argument. */
    }

  if (new_pos > fh->u.il.len)
    return -1;			/* Beyond end of file. */

  fh->u.il.pos = new_pos;
  return 0;
}

vfs_size_t
vfs_inline_size (struct vfs_file_handle_t *fh)
{
  return fh->u.il.len;
}
#endif	/* VFS_TEENSY */
