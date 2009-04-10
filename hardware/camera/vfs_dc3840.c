/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include "core/vfs/vfs.h"
#include "hardware/camera/dc3840.h"

struct vfs_file_handle_t *
vfs_dc3840_open (const char *filename)
{
  if (strcmp_P (filename, PSTR ("dc3840")))
    return NULL;		/* We have only one file to serve :) */

  /* Ask camera for a snapshot. */
  //if (dc3840_capture ())
  //  return NULL;		/* Failed to aquire image. */

  /* The camera has taken a picture, create a handle. */
  struct vfs_file_handle_t *fh = malloc (sizeof (struct vfs_file_handle_t));
  if (fh == NULL)
    return NULL;

  fh->fh_type = VFS_DC3840;
  fh->u.dc3840.pos = 0;
  return fh;
}

void
vfs_dc3840_close (struct vfs_file_handle_t *fh)
{
  free (fh);
}

vfs_size_t
vfs_dc3840_read (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  if (dc3840_data_length)
    {				/* We already know the image's total size. */
      uint16_t maxlen = dc3840_data_length - fh->u.dc3840.pos;
      if (length > maxlen) length = maxlen;
    }

  if (dc3840_get_data (buf, fh->u.dc3840.pos, length))
    return 0;			/* Failed. */

  fh->u.dc3840.pos += length;
  return length;
}
