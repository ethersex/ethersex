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
#include <ctype.h>

#include "core/vfs/vfs.h"
#include "vfs_eeprom_raw.h"
#include "hardware/i2c/master/i2c_24CXX.h"
#include "config.h"

struct vfs_file_handle_t *
vfs_eeprom_raw_open (const char *filename)
{
  if (isdigit(filename[0])) {
    struct vfs_file_handle_t *fh = malloc (sizeof (struct vfs_file_handle_t));
    if (fh == NULL)
      return NULL;

    fh->fh_type = VFS_EEPROM_RAW;
    fh->u.ee_raw.inode = atoi(filename);
    fh->u.ee_raw.end = 0;

    return fh;
  }
  return NULL;			/* File not found. */
}

void vfs_eeprom_raw_close (struct vfs_file_handle_t *t) 
{
  free(t);
}

vfs_size_t
vfs_eeprom_raw_read (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  if (fh->u.ee_raw.end) return 0;
  if (length > SFS_PAGE_SIZE) 
    length = SFS_PAGE_SIZE;
  length = i2c_24CXX_read_block(fh->u.ee_raw.inode * SFS_PAGE_SIZE, buf, length);
  return length;
}

vfs_size_t
vfs_eeprom_raw_write (struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  if (length > SFS_PAGE_SIZE) 
    length = SFS_PAGE_SIZE;
  length = i2c_24CXX_write_block(fh->u.ee_raw.inode * SFS_PAGE_SIZE, buf, length);
  return length;
}

