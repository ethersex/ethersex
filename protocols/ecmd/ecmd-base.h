/*
 *
 * (c) 2009 by Peter Marschall <peter@adpm.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#ifndef _ECMD_BASE_H
#define _ECMD_BASE_H

/* allow declaring parameters as unused */
#define _unused_	__attribute__((unused))


/* definitions and macros for ECMD backend function results */

/* function successful, no output; caller may print "OK" */
#define ECMD_FINAL_OK		0

/* function successful, output of size len in output buffer */
#define ECMD_FINAL(len)		(len)

/* "magic" indicator for continuation; dont use directly */
#define _ECMD_AGAIN_MAGIC	-10

/* output of size len in output bufferr; have caller call the function again */
/* may be used to determine len: x = ECMD_AGAIN(len) <=> len = ECMD_AGAIN(x) */
#define ECMD_AGAIN(len)		(_ECMD_AGAIN_MAGIC - (len))

/* Put this at the output at position output[returnvalue]  (byte after your last byte */
#define ECMD_NO_NEWLINE         0x23

/* error codes; requirement: ECMD_AGAIN_MAGIC < error code < 0 */
#define ECMD_ERR_PARSE_ERROR	-1	/* parse error */
#define ECMD_ERR_READ_ERROR	-2	/* reading data failed */
#define ECMD_ERR_WRITE_ERROR	-3	/* writing data failed */


/* macros to check the function results */

/* Does the function want to be called again ? */
#define is_ECMD_AGAIN(len)	(len <= _ECMD_AGAIN_MAGIC)

/* Did the function finally end successfully ? */
#define is_ECMD_FINAL(len)	(len >= 0)

/* Did the function fail */
#define is_ECMD_ERR(len)	(len > _ECMD_AGAIN_MAGIC && len < 0)

/* Magic value to use cmd buffer for state tracking */
#define ECMD_STATE_MAGIC	23

#endif /* _ECMD_BASE_H */
