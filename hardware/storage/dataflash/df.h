/*
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef _DATAFLASH_H
#define _DATAFLASH_H

#include <stdint.h>
#include "config.h"

/* for AT45DB161B (older revision, without the low-speed mode,
 * define SUPPORT_AT45DB161B */

/* commands */
#define DATAFLASH_MAIN_MEMORY_PAGE_READ 0xD2

/* check for at45db161 revision */
#ifdef SUPPORT_AT45DB161B /* use high-speed mode, no low-speed mode available */
#   define DATAFLASH_READ_BUFFER1 0xD4
#   define DATAFLASH_READ_BUFFER2 0xD6
#else /* default, current revision d, with low speed mode */
#   define DATAFLASH_READ_BUFFER1 0xD1
#   define DATAFLASH_READ_BUFFER2 0xD3
#endif

#define DATAFLASH_WRITE_BUFFER1 0x84
#define DATAFLASH_WRITE_BUFFER2 0x87
#define DATAFLASH_LOAD_BUFFER1 0x53
#define DATAFLASH_LOAD_BUFFER2 0x55
#define DATAFLASH_SAVE_BUFFER1 0x83
#define DATAFLASH_SAVE_BUFFER2 0x86
#define DATAFLASH_READ_STATUS 0xD7
#define DATAFLASH_PAGE_ERASE 0x81

/* status register bits */
#define DATAFLASH_STATUS_PAGESIZE 0
#define DATAFLASH_STATUS_PROTECT 1
#define DATAFLASH_STATUS_COMP 6
#define DATAFLASH_STATUS_BUSY 7

/* status register static bits */
#define DATAFLASH_STATUS_STATIC 0x2c

/* structures */

typedef enum { DF_BUF1 = 0, DF_BUF2 = 1 } df_buf_t;
typedef void* df_chip_t;
typedef uint16_t df_page_t;
typedef uint16_t df_size_t;
typedef uint8_t df_status_t;

/* prototypes */
void df_init(df_chip_t);
void df_buf_load(df_chip_t, df_buf_t, df_page_t);
void df_buf_read(df_chip_t, df_buf_t, void*, df_size_t, df_size_t);
void df_buf_write(df_chip_t, df_buf_t, void*, df_size_t, df_size_t);
void df_buf_save(df_chip_t, df_buf_t, df_page_t);
void df_flash_read(df_chip_t, df_page_t, void*, df_size_t, df_size_t);
void df_erase(df_chip_t, df_page_t);
df_status_t df_status(df_chip_t);
void df_wait(df_chip_t);

#endif /* _DATAFLASH_H */
