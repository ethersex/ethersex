/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
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


#ifndef DC3840_H
#define DC3840_H

/* DC3840 command set (0xFF, 0xFF, 0xFF, 0xnn) */
#define DC3840_CMD_INITIAL	1
#define DC3840_CMD_GET_PICTURE	4
#define DC3840_CMD_SNAPSHOT	5
#define DC3840_CMD_SAVE_DATA	6
#define DC3840_CMD_RESET	8
#define DC3840_CMD_POWER_OFF	9
#define DC3840_CMD_DATA		10
#define DC3840_CMD_SYNC		13
#define DC3840_CMD_ACK		14
#define DC3840_CMD_NAK		15
#define DC3840_CMD_COMPRESS	16
#define DC3840_CMD_LIGHT	18
#define DC3840_CMD_COLOR	23
#define DC3840_CMD_DIG_ZOOM	24

/* Initialize (0x01) */
#define DC3840_PREVIEW_GRAY4	1
#define DC3840_PREVIEW_GRAY16	2
#define DC3840_PREVIEW_GRAY256	3
#define DC3840_PREVIEW_COLOR8	4
#define DC3840_PREVIEW_COLOR12	5
#define DC3840_PREVIEW_COLOR16	6
#define DC3840_PREVIEW_JPEG	7

/* Get Picture (0x04) */
#define DC3840_PICT_TYPE_SNAPSHOT	1
#define DC3840_PICT_TYPE_PREVIEW	2
#define DC3840_PICT_TYPE_SERIAL_FLASH	3
#define DC3840_PICT_TYPE_PARALLEL_FLASH	4
#define DC3840_PICT_TYPE_JPEG_PREVIEW	5
#define DC3840_PICT_TYPE_DISPLAY	6

/* RESET (0x08) */
#define DC3840_RESET_COMPLETE	0
#define DC3840_RESET_STATES	1

/* Data (0x10) */
#define DC3840_DATA_TYPE_REGISTER	0
#define DC3840_DATA_TYPE_JPEG		1
#define DC3840_DATA_TYPE_PREVIEW	2
#define DC3840_DATA_TYPE_SERIAL_FLASH	3
#define DC3840_DATA_TYPE_PARALLEL_FLASH	4
#define DC3840_DATA_TYPE_JPEG_PREVIEW	5
#define DC3840_DATA_TYPE_DISPLAY	6

/* Compression (0x10) */
#define DC3840_COMPRESS_LOW	1
#define DC3840_COMPRESS_HIGH	2

/* Light (0x12) */
#define DC3840_LIGHT_DAY	0
#define DC3840_LIGHT_NIGHT	1

/* Color (0x17) */
#define DC3840_COLOR_RGB	0
#define DC3840_COLOR_BW		1



/* Initialize DC3840 communication. */
void dc3840_init (void);

/* Capture one image.  Return 0 on success. */
uint8_t dc3840_capture (void);

/* The size (in bytes) of the current image.  If known, 0 otherwise. */
extern uint16_t dc3840_data_length;

/* Store LEN bytes of image data to DATA, starting with OFFSET */
uint8_t dc3840_get_data (uint8_t *data, uint16_t offset, uint16_t len);

/* debugging support */
#define DC3840_DEBUG(a, ...) debug_printf("dc3840: " a, ## __VA_ARGS_)

#endif	/* DC3840_H */
