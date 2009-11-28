/* 
 * Copyright(C) 2009 Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "core/vfs/vfs.h"

struct vfs_file_handle_t *
vfs_host_open (const char *filename)
{
  gchar **tokens = g_strsplit (filename, "/", 0);
  gchar **out = tokens;
  int depth = 0;

  for (gchar **ptr = tokens; *ptr != NULL; ptr ++)
    {
      if (strcmp (*ptr, "..") == 0)
	{
	  if (depth)
	    {
	      depth --;
	      out --;
	    }
	  else
	    {
	      g_strfreev (tokens);
	      return NULL;
	    }
	}
      else if (strcmp (*ptr, ".") == 0)
	{
	  /* just ignore it */
	}
      else
	{
	  *out = *ptr;
	  out ++;
	  depth ++;
	}
    }

  *out = NULL;

  gchar *p = g_strjoinv ("/", tokens);
  g_strfreev (tokens);

  int fd = open (p, O_RDONLY);
  g_free (p);

  if (fd < 0)
    return NULL;

  struct vfs_file_handle_t *fh = g_malloc (sizeof (struct vfs_file_handle_t));
  fh->fh_type = VFS_HOST;
  fh->u.host.fd = fd;

  return fh;
}

void
vfs_host_close (struct vfs_file_handle_t *fh)
{
  close (fh->u.host.fd);
  g_free (fh);
}

vfs_size_t 
vfs_host_read  (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  return read (fh->u.host.fd, buf, length);
}

/* vfs_size_t 
vfs_host_size (struct vfs_file_handle_t *)
{
  return 0;
} */

uint8_t 
vfs_host_fseek (struct vfs_file_handle_t *fh, vfs_size_t offset, uint8_t whence)
{
  off_t o = lseek (fh->u.host.fd, offset, whence);
  if (o == (off_t) -1) return -1;
  return 0;
}
