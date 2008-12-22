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

#ifdef VFS_SD_SUPPORT

struct vfs_file_handle_t *
vfs_sd_open (const char *filename)
{
  struct fat_file_struct* i = open_file_in_dir(fat_fs, sd_cwd, filename);

  if (i == NULL)
    return NULL;		/* No such file. */

  struct vfs_file_handle_t *fh = malloc (sizeof (struct vfs_file_handle_t));
  if (fh == NULL)
    return NULL;

  fh->fh_type = VFS_SD;
  fh->u.sd = i;

  return fh;
}

void
vfs_sd_close (struct vfs_file_handle_t *fh)
{
  fat_close_file (fh->u.sd);
  free (fh);
}

vfs_size_t
vfs_sd_read (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  return fat_read_file (fh->u.sd, buf, length);
}

vfs_size_t
vfs_sd_write (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  return fat_write_file (fh->u.sd, buf, length);
}

uint8_t
vfs_sd_fseek (struct vfs_file_handle_t *fh, vfs_size_t offset,
	      uint8_t whence)
{
  if (fat_seek_file (fh->u.sd, &offset, whence) == 0)
    return -1;			/* Fail. */
  else
    return 0;
}


uint8_t
vfs_sd_truncate (struct vfs_file_handle_t *fh, vfs_size_t length)
{
  return fat_resize_file (fh->u.sd, length) == 0;
}


struct vfs_file_handle_t *
vfs_sd_create (const char *name)
{
  struct fat_dir_entry_struct file_entry;
  fat_create_file (sd_cwd, name, &file_entry);

  struct vfs_file_handle_t *fh = vfs_sd_open (name);

  if (fh && vfs_sd_size (fh) != 0)
    vfs_sd_truncate (fh, 0);

  return fh;
}


fs_size_t
vfs_sd_size (struct vfs_file_handle_t *fh)
{
  return fh->u.sd->dir_entry.file_size;
}

#endif	/* VFS_SD_SUPPORT */
