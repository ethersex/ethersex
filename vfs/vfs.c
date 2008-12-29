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

#include "vfs.h"
#ifndef VFS_TEENSY

struct vfs_func_t vfs_funcs[] = {
#ifdef VFS_EEPROM_SUPPORT
  VFS_EEPROM_FUNCS,
#endif
#ifdef VFS_EEPROM_RAW_SUPPORT
  VFS_EEPROM_RAW_FUNCS,
#endif
#ifdef VFS_DF_SUPPORT
  VFS_DF_FUNCS,
#endif
#ifdef VFS_DF_RAW_SUPPORT
  VFS_DF_RAW_FUNCS,
#endif
#ifdef VFS_SD_SUPPORT
  VFS_SD_FUNCS,
#endif
#ifdef VFS_PROC_SUPPORT
  VFS_PROC_FUNCS,
#endif
#ifdef VFS_INLINE_SUPPORT
  VFS_INLINE_FUNCS,
#endif
};

struct vfs_file_handle_t *
vfs_open (const char *filename)
{
  struct vfs_file_handle_t *fh = NULL;

  for (uint8_t i = 0; fh == NULL && i < VFS_LAST; i ++)
    fh = vfs_funcs[i].open (filename);

  return fh;
}

struct vfs_file_handle_t *
vfs_create (const char *name)
{
  struct vfs_file_handle_t *fh = NULL;

  for (uint8_t i = 0; fh == NULL && i < VFS_LAST; i ++)
    if (vfs_funcs[i].create)
      fh = vfs_funcs[i].create (name);

  return fh;
}

#endif	/* not VFS_TEENSY */
