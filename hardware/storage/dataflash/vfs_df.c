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

#include <stdlib.h>

#include "core/vfs/vfs.h"

struct vfs_file_handle_t *
vfs_df_open (const char *filename)
{
  fs_inode_t i = fs_get_inode (&fs, filename);

  if (i == 0xffff)
    return NULL;		/* No such file. */

  struct vfs_file_handle_t *fh = malloc (sizeof (struct vfs_file_handle_t));
  if (fh == NULL)
    return NULL;

  fh->fh_type = VFS_DF;
  fh->u.df.inode = i;
  fh->u.df.offset = 0;

  return fh;
}

void
vfs_df_close (struct vfs_file_handle_t *fh)
{
  free (fh);
}

vfs_size_t
vfs_df_read (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  vfs_size_t ret = fs_read (&fs, fh->u.df.inode, buf, fh->u.df.offset, length);

  /* Read was successful, update offset. */
  if (ret > 0) fh->u.df.offset += ret;

  return ret;
}

vfs_size_t
vfs_df_write (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  fs_status_t i = fs_write (&fs, fh->u.df.inode, buf, fh->u.df.offset, length);

  if (i == FS_OK)
    {
      fh->u.df.offset += length;
      return length;
    }
  else
    {
      return 0;			/* Fail. */
    }
}

uint8_t
vfs_df_fseek (struct vfs_file_handle_t *fh, vfs_size_t offset,
	      uint8_t whence)
{
  fs_size_t len = fs_size (&fs, fh->u.df.inode);
  fs_size_t new_pos;

  switch (whence)
    {
    case SEEK_SET:
      new_pos = offset;
      break;

    case SEEK_CUR:
      new_pos = fh->u.df.offset + offset;
      break;

    case SEEK_END:
      new_pos = len + offset;
      break;

    default:
      return -1;		/* Invalid argument. */
    }

  if (new_pos > len)
    return -1;			/* Beyond end of file.
				   FIXME: we could increase file size. */

  fh->u.df.offset = new_pos;
  return 0;

}

uint8_t
vfs_df_truncate (struct vfs_file_handle_t *fh, vfs_size_t length)
{
  return fs_truncate (&fs, fh->u.df.inode, length);
}


struct vfs_file_handle_t *
vfs_df_create (const char *name)
{
  fs_create (&fs, name);

  struct vfs_file_handle_t *fh = vfs_df_open (name);

  if (fh && vfs_df_size (fh) != 0)
    vfs_df_truncate (fh, 0);

  return fh;
}


uint8_t
vfs_df_unlink (const char *name)
{
  return fs_remove (&fs, name);
}

vfs_size_t
vfs_df_size (struct vfs_file_handle_t *fh)
{
  return fs_size (&fs, fh->u.df.inode);
}
