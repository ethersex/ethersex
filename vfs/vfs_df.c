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

#include "vfs.h"

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
  fh->u.df = i;

  return fh;
}


vfs_size_t
vfs_df_read (struct vfs_file_handle_t *fh, void *buf,
	     vfs_size_t offset, vfs_size_t length)
{
  return fs_read (&fs, fh->u.df, buf, offset, length);
}

vfs_size_t
vfs_df_write (struct vfs_file_handle_t *fh, void *buf,
	      vfs_size_t offset, vfs_size_t length)
{
  fs_status_t i = fs_write (&fs, fh->u.df, buf, offset, length);
  return i == FS_OK ? length : 0;
}

uint8_t
vfs_df_truncate (struct vfs_file_handle_t *fh, vfs_size_t length)
{
  return fs_truncate (&fs, fh->u.df, length);
}

uint8_t
vfs_df_create (const char *name)
{
  return fs_create (&fs, name);
}
