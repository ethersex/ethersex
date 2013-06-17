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

#include "config.h"

#include "hardware/storage/sd_reader/sd_raw.h"
#include "hardware/storage/sd_reader/partition.h"
#include "hardware/storage/sd_reader/fat.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif

struct partition_struct *sd_active_partition;

#if SD_RAW_WRITE_SUPPORT == 1
#define sd_try_open_partition(partno)				\
  partition_open (sd_raw_read, sd_raw_read_interval,		\
		  sd_raw_write, sd_raw_write_interval, partno)
#else
#define sd_try_open_partition(partno)				\
  partition_open (sd_raw_read, sd_raw_read_interval,		\
		  NULL, 0, partno)
#endif

uint8_t
sd_try_init (void)
{
  if (sd_raw_init () != 1)
    return 1;			/* Low-level init failed. */

  if ((sd_active_partition = sd_try_open_partition (0)) == NULL &&
      (sd_active_partition = sd_try_open_partition (-1)) == NULL)
    {
      SDDEBUGRAW ("initialized SD-Card, but cannot open partition\n");
      return 1;
    }

  return 0;
}
