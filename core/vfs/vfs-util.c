/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2010 by Christian Dietrich <stettberger@dokucode.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include "core/vfs/vfs.h"
#include "core/vfs/vfs-util.h"

#ifndef VFS_TEENSY

#ifdef ETHERNET_SUPPORT
#include "protocols/uip/uip.h"	/* for uip_buf */

uint8_t
vfs_copy_file (const char *dest, const char *src)
{
  struct vfs_file_handle_t *d = vfs_creat (dest);
  if (d == NULL)
    {
      /* Failed to create destination file. */
      return 1;
    }

  struct vfs_file_handle_t *s = vfs_open (src);
  if (s == NULL)
    {
      /* Failed to open source file. */
      vfs_close (d);
      return 1;
    }

  uint16_t i;
  while ((i = vfs_read (s, uip_buf, sizeof (uip_buf))))
    {
      uint16_t j = vfs_write (d, uip_buf, i);

      if (i != j)		/* Short write */
	{
	  vfs_close (s);
	  vfs_close (d);
	  return 1;		/* TODO Shall we delete 'dest'? */
	}

      if (i < sizeof (uip_buf))	/* EOF */
	break;

      wdt_kick ();
    }

  vfs_close (s);
  vfs_close (d);

  return 0;
}

#endif /* ETHERNET_SUPPORT */


struct vfs_file_handle_t *
vfs_open_or_creat(const char* filename)
{
    struct vfs_file_handle_t * fd = vfs_open(filename);
    if (! fd ) {
	/* Perhaps the file wasn't existing */
	fd = vfs_create(filename);
    }

    return fd; /* Here fd may be NULL or a filehandle */
}

uint8_t 
vfs_file_append(const char *filename, uint8_t* buf, uint16_t len)
{
    struct vfs_file_handle_t *fd = vfs_open_or_creat(filename);
    if (fd) {
	vfs_fseek(fd, 0, SEEK_END);
	vfs_write(fd, buf, len);
	vfs_close(fd);
	return 1;
    }
    return 0;
}

#endif  /* VFS_TEENSY */
