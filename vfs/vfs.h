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

#ifndef VFS_H
#define VFS_H

#include "../config.h"

enum vfs_type_t {
#ifdef VFS_EEPROM_SUPPORT
  VFS_EEPROM,
#endif
#ifdef VFS_EEPROM_RAW_SUPPORT
  VFS_EEPROM_RAW,
#endif
#ifdef VFS_DF_SUPPORT
  VFS_DF,
#endif
#ifdef VFS_DF_RAW_SUPPORT
  VFS_DF_RAW,
#endif
#ifdef VFS_SD_SUPPORT
  VFS_SD,
#endif
#ifdef VFS_PROC_SUPPORT
  VFS_PROC,
#endif
#ifdef VFS_INLINE_SUPPORT
  VFS_INLINE,
#endif

  VFS_LAST
};

/* Forward declarations. */
struct vfs_file_handle_t;


/* VFS-related types. */
#include <stdint.h>

#ifdef VFS_TEENSY
/* Only VFS_INLINE is enabled, 16-bit should be enough xD */
typedef uint16_t vfs_size_t;
#else
typedef uint32_t vfs_size_t;
#endif

#include "vfs_df.h"
#include "vfs_sd.h"
#include "vfs_inline.h"
#include "vfs_eeprom_raw.h"

struct vfs_file_handle_t {
  /* The vfs_type_t of the VFS module that is responsible for this
     file handle. */
  uint8_t fh_type;

  union {
    vfs_file_handle_df_t df;
    vfs_file_handle_sd_t sd;
    vfs_file_handle_inline_t il;
    vfs_file_handle_eeprom_raw_t ee_raw;
  } u;
};

struct vfs_func_t {
  /* VFS module name, i.e. the "mount point" */
  const char *mod_name;

  /* Try to open the already existing file named FILENAME, return a
     handle or NULL on error. */
  struct vfs_file_handle_t * (*open) (const char *filename);

  /* Close the referenced file. */
  void (*close) (struct vfs_file_handle_t *);

  /* Read LENGTH bytes to the memory referenced by BUF.
     Returns the number of bytes actually read. */
  vfs_size_t (*read) (struct vfs_file_handle_t *, void *buf,
		      vfs_size_t length);

  /* Write LENGTH bytes stored at BUF.  */
  vfs_size_t (*write) (struct vfs_file_handle_t *, void *buf,
		       vfs_size_t length);

  /* Reposition the stream to OFFSET, considering WHENCE,
     where whence is either SEEK_SET, SEEK_CUR or SEEK_END */
  uint8_t (*fseek) (struct vfs_file_handle_t *, vfs_size_t offset,
		    uint8_t whence);

  /* Truncate the file to LENGTH bytes.  Return 0 on success. */
  uint8_t (*truncate) (struct vfs_file_handle_t *, vfs_size_t length);

  /* Try to create a new file called NAME and return a file handle.
     If the file exists already it's automatically truncated to zero size. */
  struct vfs_file_handle_t * (*create) (const char *name);

  /* Return the size of the file. */
  vfs_size_t (*size) (struct vfs_file_handle_t *);
};

extern struct vfs_func_t vfs_funcs[];

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Generic variant of open that automagically finds the suitable
   VFS module. */
struct vfs_file_handle_t *vfs_open (const char *filename);

/* Generic variante of create, that automatically finds a suitable
   store for the new file. */
struct vfs_file_handle_t *vfs_create (const char *name);


/* Generation of forwarder functions. */
#define VFS_REDIR(call,def,handle,args...)	      \
  ((vfs_funcs[handle->fh_type].call)		      \
   ? vfs_funcs[handle->fh_type].call(handle, ##args)  \
   : def)

#define vfs_close(handle)       VFS_REDIR(close, 0, handle)
#define vfs_read(handle...)     VFS_REDIR(read, 0, handle)
#define vfs_write(handle...)    VFS_REDIR(write, 0, handle)
#define vfs_fseek(handle...)    VFS_REDIR(fseek, -1, handle)
#define vfs_truncate(handle...) VFS_REDIR(truncate, 1, handle)
#define vfs_size(handle...)     VFS_REDIR(size, -1, handle)

#define vfs_rewind(handle)      vfs_fseek(handle, 0, SEEK_SET)

/* Synonym to be more like posix :) */
#define vfs_creat(n...)		vfs_create(n)


#ifdef VFS_TEENSY
#include "vfs_teensy.h"
#endif

#endif	/* VFS_H */
