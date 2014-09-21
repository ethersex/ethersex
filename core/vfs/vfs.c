/*
 * Copyright (c) 2008-2009 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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
#include "core/debug.h"
#include "core/vfs/vfs.h"
#ifndef VFS_TEENSY

const struct vfs_func_t vfs_funcs[] PROGMEM = {
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
#ifdef VFS_DC3840_SUPPORT
  VFS_DC3840_FUNCS,
#endif
#ifdef VFS_HOST_SUPPORT
  VFS_HOST_FUNCS,
#endif
};


struct vfs_file_handle_t *
vfs_open(const char *filename)
{
  struct vfs_file_handle_t *fh = NULL;
  for (uint8_t i = 0; fh == NULL && i < VFS_LAST; i++)
  {
    struct vfs_func_t funcs;
    memcpy_P(&funcs, &vfs_funcs[i], sizeof(struct vfs_func_t));
    if (funcs.open)
      fh = funcs.open(filename);
  }

  return fh;
}

struct vfs_file_handle_t *
vfs_create(const char *name)
{
  struct vfs_file_handle_t *fh = NULL;
  for (uint8_t i = 0; fh == NULL && i < VFS_LAST; i++)
  {
    struct vfs_func_t funcs;
    memcpy_P(&funcs, &vfs_funcs[i], sizeof(struct vfs_func_t));
    if (funcs.create)
      fh = funcs.create(name);
  }

  return fh;
}

/* flag: 0=read, 1=write, 2=size */
vfs_size_t
vfs_read_write_size(uint8_t flag, struct vfs_file_handle_t * handle,
                    void *buf, vfs_size_t length)
{
  struct vfs_func_t funcs;
  memcpy_P(&funcs, &vfs_funcs[handle->fh_type], sizeof(struct vfs_func_t));

  if (flag == 0 && funcs.read)
    return funcs.read(handle, buf, length);

  if (flag == 1 && funcs.write)
    return funcs.write(handle, buf, length);

  if (flag == 2 && funcs.size)
    return funcs.size(handle);

  return 0;
}

/* flag: 0=fseek, 1=truncate, 2=close */
uint8_t
vfs_fseek_truncate_close(uint8_t flag, struct vfs_file_handle_t * handle,
                         vfs_size_t length, uint8_t whence)
{
  struct vfs_func_t funcs;
  memcpy_P(&funcs, &vfs_funcs[handle->fh_type], sizeof(struct vfs_func_t));

  if (flag == 0 && funcs.fseek)
    /* handle, offset, whence */
    return funcs.fseek(handle, length, whence);

  if (flag == 1 && funcs.truncate)
    return funcs.truncate(handle, length);

  if (flag == 2 && funcs.close)
    funcs.close(handle);

  return 0;
}

uint8_t
vfs_unlink(const char *name)
{
  uint8_t retval = 1;
  for (uint8_t i = 0; retval != 0 && i < VFS_LAST; i++)
  {
    struct vfs_func_t funcs;
    memcpy_P(&funcs, &vfs_funcs[i], sizeof(struct vfs_func_t));
    if (funcs.unlink)
      retval = funcs.unlink(name);
  }

  return retval;
}

#endif /* not VFS_TEENSY */

/*
  -- Ethersex META --
  header(core/vfs/vfs.h)
*/
