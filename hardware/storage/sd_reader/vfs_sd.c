/*
 * Copyright (c) 2008,2009 Stefan Siegl <stesie@brokenpipe.de>
 * Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "hardware/storage/sd_reader/fat.h"
#include "hardware/storage/sd_reader/sd_raw.h"
#include "hardware/storage/sd_reader/partition.h"
#include "core/vfs/vfs.h"
#include "core/debug.h"

static struct fat_fs_struct *vfs_sd_fat;
struct fat_dir_struct *vfs_sd_rootnode;

struct fat_dir_struct *
vfs_sd_chdir(const char *dirname)
{
  struct fat_dir_entry_struct handle;
  if (fat_get_dir_entry_of_path(vfs_sd_fat, dirname, &handle) == 0)
    return NULL;

  return fat_open_dir(vfs_sd_fat, &handle);
}

uint8_t
vfs_sd_try_open_rootnode(void)
{
  if ((vfs_sd_fat = fat_open(sd_active_partition)) == NULL)
  {
    SDDEBUGVFS("fat_open failed\n");
  }
  else
  {
    if ((vfs_sd_rootnode = vfs_sd_chdir("/")) != NULL)
    {
      SDDEBUGVFS("card initialized and root node opened\n");
      return 0;                 /* Jippie, we're set. */
    }

    SDDEBUGVFS("vfs_sd_chdir failed\n");
  }

  SDDEBUGVFS("card initialized, but failed to open root node\n");

#ifdef HAVE_SD_READER_POWERON
  PIN_CLEAR(SD_READER_POWERON);
  _delay_ms(100);
  PIN_SET(SD_READER_POWERON);
  _delay_ms(50);
#endif

  return 1;
}

static struct fat_dir_struct *
vfs_sd_traverse(const char *path, const char **basename)
{
  while (*path == '/')
    path++;
  *basename = path;

  struct fat_dir_struct *parent = vfs_sd_rootnode;
  char *sep = strrchr(path, '/');

  if (sep)
  {
    char *dirname = strdup(path);
    if (dirname == NULL)
      return NULL;
    dirname[sep - path] = 0;
    parent = vfs_sd_chdir(dirname);
    free(dirname);
    *basename = sep + 1;
  }

  return parent;
}

#if SD_WRITE_SUPPORT == 1
static struct vfs_file_handle_t *
vfs_sd_open_in(struct fat_dir_struct *parent, const char *filename)
{
  fat_reset_dir(parent);

  struct fat_dir_entry_struct filep;
  while (fat_read_dir(parent, &filep))
  {
    if (strcmp(filep.long_name, filename))
      continue;

    if (filep.attributes & FAT_ATTRIB_DIR)
      return NULL;              /* Is a directory. */

    /* Got it :) */
    struct fat_file_struct *inode = fat_open_file(vfs_sd_fat, &filep);
    struct vfs_file_handle_t *fh = malloc(sizeof(struct vfs_file_handle_t));
    if (fh == NULL)
      return NULL;

    fh->fh_type = VFS_SD;
    fh->u.sd = inode;

    return fh;
  }

  return NULL;                  /* No such file. */
}
#endif

void
vfs_sd_close(struct vfs_file_handle_t *fh)
{
  fat_close_file(fh->u.sd);
  free(fh);
}

vfs_size_t
vfs_sd_read(struct vfs_file_handle_t *fh, void *buf, vfs_size_t length)
{
  return fat_read_file(fh->u.sd, buf, length);
}

#if SD_WRITE_SUPPORT == 1
vfs_size_t
vfs_sd_write(struct vfs_file_handle_t * fh, void *buf, vfs_size_t length)
{
  return fat_write_file(fh->u.sd, buf, length);
}
#endif

uint8_t
vfs_sd_fseek(struct vfs_file_handle_t * fh, vfs_size_t offset, uint8_t whence)
{
  return fat_seek_file(fh->u.sd, &offset, whence) == 0;
}

#if SD_WRITE_SUPPORT == 1
uint8_t
vfs_sd_truncate(struct vfs_file_handle_t * fh, vfs_size_t length)
{
  return fat_resize_file(fh->u.sd, length) == 0;
}
#endif

static struct vfs_file_handle_t *
vfs_sd_create_open(const char *name, uint8_t create)
{
  const char *basename;
  struct fat_dir_struct *parent = vfs_sd_traverse(name, &basename);
  if (!parent)
    return NULL;

#if SD_WRITE_SUPPORT == 1
  if (create)
  {
    struct fat_dir_entry_struct file_entry;
    fat_create_file(parent, basename, &file_entry);
  }
#endif

  struct vfs_file_handle_t *fh = vfs_sd_open_in(parent, basename);

#if SD_WRITE_SUPPORT == 1
  if (create && fh && vfs_sd_size(fh) != 0)
    vfs_sd_truncate(fh, 0);
#endif

  if (parent != vfs_sd_rootnode)
    fat_close_dir(parent);

  return fh;
}

#if SD_WRITE_SUPPORT == 1
struct vfs_file_handle_t *
vfs_sd_create(const char *name)
{
  return vfs_sd_create_open(name, 1);
}
#endif


struct vfs_file_handle_t *
vfs_sd_open(const char *name)
{
  return vfs_sd_create_open(name, 0);
}

#if SD_WRITE_SUPPORT == 1
uint8_t
vfs_sd_mkdir_recursive(const char *path)
{
  struct fat_dir_entry_struct handle = {.attributes = FAT_ATTRIB_DIR };
  handle.attributes = FAT_ATTRIB_DIR;

recurse_loop:
  while (*path == '/')
    path++;

  struct fat_dir_struct *dir = fat_open_dir(vfs_sd_fat, &handle);
  if (dir == NULL)
  {
    SDDEBUGVFS("fat_open_dir failed\n");
    return 1;
  }

  /* Extract directory name (especially it's length) regarding
   * the next level. */
  char *ptr = strchr(path, '/');
  uint8_t l;

  if (ptr)
    l = ptr - path;
  else
    l = strlen(path);

  while (fat_read_dir(dir, &handle))
  {
    if (strncmp(handle.long_name, path, l))
      continue;                 /* Mismatch. */

    /* Found directory, recurse. */
    fat_close_dir(dir);         /* Close parent handle. */

    if (!(handle.attributes & FAT_ATTRIB_DIR))
    {
      SDDEBUGVFS("'%s' isn't a directory.\n", path);
      return 1;
    }

    path += l;
    if (*path == 0)
      return 0;                 /* Target already exists. */

    goto recurse_loop;          /* Recurse down ... */
  }

  /* Sub-directory not found, too bad. Let's try to create it. */
  {
    char buf[l + 1];
    memcpy(buf, path, l);
    buf[l] = 0;

    if (!fat_create_dir(dir, buf, &handle))
    {
      fat_close_dir(dir);
      SDDEBUGVFS("failed to create dir '%s'\n", buf);
      return 1;
    }

    fat_close_dir(dir);

    path += l;
    if (*path == 0)
      return 0;                 /* Ok, that was the last piece. */

    goto recurse_loop;          /* Yippie, one more round ... */
  }
}

uint8_t
vfs_sd_unlink(const char *name)
{
  const char *basename;

  struct fat_dir_struct *dd = vfs_sd_traverse(name, &basename);
  if (dd)
  {
    struct fat_dir_entry_struct file_entry;
    while (fat_read_dir(dd, &file_entry))
    {
      if (strcmp(file_entry.long_name, basename) == 0)
      {
        fat_reset_dir(dd);
        return fat_delete_file(vfs_sd_fat, &file_entry) == 0;
      }
    }
  }

  return 1;
}
#endif

vfs_size_t
vfs_sd_size(struct vfs_file_handle_t * fh)
{
  return fh->u.sd->dir_entry.file_size;
}

#ifdef SD_PING_READ
static uint8_t
vfs_sd_ping(void)
{
  /* Don't even try to ping if we don't have a rootnode. */
  if (!vfs_sd_rootnode)
    return 1;

  uint8_t result = 0;
  unsigned char buf[5];

  if (sd_raw_read(0, buf, 5) == 0)
    result = 1;
  if (sd_raw_read(512, buf, 5) == 0)
    result = 1;

  SDDEBUGVFS("ping result=%d\n", result);
  return result;
}

static void
vfs_sd_umount(void)
{
  if (vfs_sd_rootnode)
  {
    fat_close_dir(vfs_sd_rootnode);
    vfs_sd_rootnode = NULL;
  }

  if (vfs_sd_fat)
  {
    fat_close(vfs_sd_fat);
    vfs_sd_fat = NULL;
  }

  if (sd_active_partition)
  {
    partition_close(sd_active_partition);
    sd_active_partition = NULL;
  }
}

void
vfs_sd_ping_read_periodic(void)
{
  if (vfs_sd_ping())
  {
    vfs_sd_umount();
  }
}
#endif /* SD_PING_READ */

/*
  -- Ethersex META --
  header(hardware/storage/sd_reader/vfs_sd.h)
  ifdef(`conf_SD_PING_READ', `timer(500, `vfs_sd_ping_read_periodic()')')
*/
