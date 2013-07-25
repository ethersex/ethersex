/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/pgmspace.h>
#include <stdio.h>

#include "core/debug.h"
#include "core/vfs/vfs.h"
#include "core/vfs/vfs-util.h"
#include "hardware/camera/dc3840.h"
#include "hardware/camera/dc3840-util.h"
#include "services/clock/clock.h"

#ifdef VFS_SD_SUPPORT
void
dc3840_save_snapshot (void)
{
  if (dc3840_capture ())
    return;			/* Camera failed to make a picture. */

  /* Generate destination directory name, based on current date and hour. */
  clock_datetime_t datetime;
  clock_localtime(&datetime, clock_get_time());

  char filename[24];
  sprintf_P (filename, PSTR ("%04d/%02d/%02d/%02d"), datetime.year,
	     datetime.month, datetime.day, datetime.hour);

  if (vfs_sd_mkdir_recursive (filename))
    {
      debug_printf ("dc3840_save_snapshot: can't create dir %s\n", filename);
      return;
    }

  /* Now append the filename (minute and seconds) */
  sprintf_P (filename + 13, PSTR ("/%02d%02d.jpg"), datetime.min,
	     datetime.sec);

  /* Get source-filename.  Some hackery to have it in .text section :) */
  char src[7];
  memcpy_P (src, PSTR ("dc3840"), 7);

  vfs_copy_file (filename, src);
}
#endif // VFS_SD_SUPPORT
