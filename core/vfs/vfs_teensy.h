/*
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#ifndef VFS_TEENSY_H
#define VFS_TEENSY_H

/* VFS_TEENSY mode cruft ...
 *
 * VFS_TEENSY is a teensy mode of VFS, that directly connects the VFS inline
 * cruft to VFS, i.e. tries to omit the whole layer in between. */

#undef vfs_close
#undef vfs_read
#undef vfs_write
#undef vfs_fseek
#undef vfs_truncate
#undef vfs_size
#undef vfs_rewind

#define vfs_open	vfs_inline_open
#define vfs_read	vfs_inline_read
#define vfs_close(i)	free(i)
#define vfs_fseek(fh,p,w)   (((w) == SEEK_SET) ? ((fh)->u.il.pos = (p)) : -1)
#define vfs_size(fh)	((fh)->u.il.len)
#define vfs_rewind(fh)  ((fh)->u.il.pos = 0)

#endif  /* VFS_TEENSY_H */
