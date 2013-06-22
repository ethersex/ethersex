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

#ifndef VFS_H
#define VFS_H

#include <avr/pgmspace.h>
#include "config.h"

enum vfs_type_t
{
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
#ifdef VFS_DC3840_SUPPORT
  VFS_DC3840,
#endif
#ifdef VFS_HOST_SUPPORT
  VFS_HOST,
#endif

  VFS_LAST
};

/* Forward declarations. */
struct vfs_file_handle_t;


/* VFS-related types. */
#include <stdint.h>

#if defined(VFS_TEENSY) && (FLASHEND < UINT16_MAX)
/* Only VFS_INLINE is enabled, 16-bit should be enough xD */
typedef uint16_t vfs_size_t;
#else
typedef uint32_t vfs_size_t;
#endif

#include "hardware/storage/dataflash/vfs_df.h"
#include "hardware/storage/sd_reader/vfs_sd.h"
#include "core/vfs/vfs_inline.h"
#include "hardware/i2c/master/vfs_eeprom.h"
#include "hardware/i2c/master/vfs_eeprom_raw.h"
#include "hardware/camera/vfs_dc3840.h"
#include "core/host/vfs.h"

struct vfs_file_handle_t
{
  /* The vfs_type_t of the VFS module that is responsible for this
   * file handle. */
  uint8_t fh_type;

  union
  {
    vfs_file_handle_eeprom_t ee;
    vfs_file_handle_eeprom_raw_t ee_raw;
    vfs_file_handle_df_t df;
    vfs_file_handle_sd_t sd;
    vfs_file_handle_inline_t il;
    vfs_file_handle_dc3840_t dc3840;
    vfs_file_handle_host_t host;
  } u;
};

struct vfs_func_t
{
  /* VFS module name, i.e. the "mount point" */
  const char *mod_name;

  /* Try to open the already existing file named FILENAME, return a
   * handle or NULL on error. */
  struct vfs_file_handle_t *(*open) (const char *filename);

  /* Close the referenced file. */
  void (*close) (struct vfs_file_handle_t *);

  /* Read LENGTH bytes to the memory referenced by BUF.
   * Returns the number of bytes actually read. */
    vfs_size_t(*read) (struct vfs_file_handle_t *, void *buf,
                       vfs_size_t length);

  /* Write LENGTH bytes stored at BUF.  */
    vfs_size_t(*write) (struct vfs_file_handle_t *, void *buf,
                        vfs_size_t length);

  /* Reposition the stream to OFFSET, considering WHENCE,
   * where whence is either SEEK_SET, SEEK_CUR or SEEK_END */
    uint8_t(*fseek) (struct vfs_file_handle_t *, vfs_size_t offset,
                     uint8_t whence);

  /* Truncate the file to LENGTH bytes. Return 0 on success. */
    uint8_t(*truncate) (struct vfs_file_handle_t *, vfs_size_t length);

  /* Try to create a new file called NAME and return a file handle.
   * If the file exists already it's automatically truncated to zero size. */
  struct vfs_file_handle_t *(*create) (const char *name);

  /* Unlink the already existing file named NAME.
   * Return 0 on success. */
    uint8_t(*unlink) (const char *name);

  /* Return the size of the file. */
    vfs_size_t(*size) (struct vfs_file_handle_t *);
};

extern const struct vfs_func_t vfs_funcs[];

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Generic variant of open that automagically finds the suitable
   VFS module. */
struct vfs_file_handle_t *vfs_open(const char *filename);

/* Generic variante of create, that automatically finds a suitable
   store for the new file. */
struct vfs_file_handle_t *vfs_create(const char *name);

uint8_t vfs_fseek_truncate_close(uint8_t flag,
                                 struct vfs_file_handle_t *handle,
                                 vfs_size_t length, uint8_t whence);

uint8_t vfs_unlink(const char *filename);

vfs_size_t vfs_read_write_size(uint8_t flag, struct vfs_file_handle_t *handle,
                               void *buf, vfs_size_t length);

#define VFS_FUNC(handle,call)	              \
  ((pgm_read_word(((void *)&(vfs_funcs[(handle)->fh_type].call))))

/* Generation of forwarder functions. */

#define VFS_HAVE_FUNC(handle,call)	              \
  (pgm_read_word(((char *)&(vfs_funcs[(handle)->fh_type].call))) != 0)

#define vfs_read(handle, buf, len)  vfs_read_write_size(0, handle, buf, len)
#define vfs_write(handle, buf, len) vfs_read_write_size(1, handle, buf, len)
#define vfs_size(handle)            vfs_read_write_size(2, handle, NULL, 0)

#define vfs_fseek(handle, offset, whence) \
   vfs_fseek_truncate_close(0, handle, offset, whence)
#define vfs_truncate(handle, length) \
  vfs_fseek_truncate_close(1, handle, length, 0)
#define vfs_close(handle) \
  vfs_fseek_truncate_close(2, handle, 0, 0)

#define vfs_rewind(handle)      vfs_fseek(handle, 0, SEEK_SET)

/* Synonym to be more like posix :) */
#define vfs_creat(n...)		vfs_create(n)


#ifdef VFS_TEENSY
#include "vfs_teensy.h"
#endif

#endif /* VFS_H */
