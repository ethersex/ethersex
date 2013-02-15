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

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    uint8_t link :1;

#ifdef BOOTLOADER_JUMP
    uint8_t request_bootloader      :1;
#endif
    uint8_t request_reset           :1;
#ifndef TEENSY_SUPPORT
    uint8_t request_wdreset         :1;
#endif
#ifdef TFTP_CRC_SUPPORT
    uint8_t verify_tftp_crc_content :1;
#endif
} global_status_t;

#ifdef BOOTLOADER_SUPPORT
extern uint16_t bootload_delay;
#endif

extern global_status_t status;

#endif /* _GLOBAL_H */
