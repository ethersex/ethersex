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
  /* fs_status_t i = fs_write (&fs, fh->u.sd, buf, offset, length);
     return i == FS_OK ? length : 0; */
  return 0;			/* TODO */
}

uint8_t
vfs_sd_truncate (struct vfs_file_handle_t *fh, vfs_size_t length)
{
  /* return fs_truncate (&fs, fh->u.sd, length); */
  return 0;			/* TODO */
}

uint8_t
vfs_sd_create (const char *name)
{
  /* return fs_create (&fs, name); */
  return 0;
}

fs_size_t
vfs_sd_size (struct vfs_file_handle_t *fh)
{
  return fh->u.sd->dir_entry.file_size;
}
