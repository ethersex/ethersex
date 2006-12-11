/* vim:fdm=marker ts=4 et ai
 * {{{
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 }}} */

#ifndef _DATAFLASH_H
#define _DATAFLASH_H

#include "config.h"
#include "common.h"


/* commands */
#define DATAFLASH_MAIN_MEMORY_PAGE_READ 0xD2
#define DATAFLASH_READ_BUFFER1 0xD1
#define DATAFLASH_READ_BUFFER2 0xD3
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

/*
  * 8bit: magic byte, 0xFF if erased, 0x23 if used
  * 32bit: node number
  * 16bit: index of this page in the node
  * 32bit: version (always increasing, only pages with same node number and
           highest version are valid, this helps providing "wear levelling")
  * 16bit: pointer to the next page for this node
  * 3 byter reserved
  */
struct filesystem_node_t {
    uint8_t magic;
    uint32_t node_number;
    uint16_t page_index;
    uint32_t version;
    uint16_t next_page;
    uint8_t reserved[3];
};

/* prototypes */
uint8_t dataflash_read_status(void);
void dataflash_wait_busy(void);
void dataflash_read_flash(uint16_t page_address, uint16_t offset, uint8_t *data, uint16_t len);
void dataflash_load_buffer(uint8_t buffer, uint16_t page_address);
void dataflash_read_buffer(uint8_t buffer, uint16_t offset, uint8_t *data, uint16_t len);
void dataflash_write_buffer(uint8_t buffer, uint16_t offset, uint8_t *data, uint16_t len);
void dataflash_save_buffer(uint8_t buffer, uint16_t page_address);
void dataflash_erase_page(uint16_t page_address);

#endif
