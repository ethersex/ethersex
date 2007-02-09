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

#ifndef _ETHCMD_MESSAGE_H
#define _ETHCMD_MESSAGE_H

#include <stdint.h>

#define packed __attribute__ ((__packed__))

enum ethcmd_subsystem_t {
    ETHCMD_SYS_VERSION = 0x00,
    ETHCMD_SYS_ONEWIRE = 0x05,
    ETHCMD_SYS_FS20 = 0x06,
    ETHCMD_SYS_STORAGE = 0x07,
    ETHCMD_SYS_RESPONSE = 0xff,
} packed;

enum ethcmd_version_cmd_t {
    ETHCMD_VERSION_REQUEST = 0x00,
    ETHCMD_VERSION_REPLY = 0x01,
} packed;

enum ethcmd_fs20_cmd_t {
    ETHCMD_FS20_SEND = 0x00,
} packed;

enum ethcmd_storage_cmd_t {
    ETHCMD_STORAGE_LIST = 0x00,
    ETHCMD_STORAGE_SEND = 0x01,
} packed;


struct ethcmd_msg_t {
    enum ethcmd_subsystem_t sys;
    uint8_t reserved[7];
} packed;

struct ethcmd_response_t {
    enum ethcmd_subsystem_t sys;
    enum ethcmd_subsystem_t old_sys;
    uint8_t status;
    uint8_t reserved[5];
} packed;

struct ethcmd_msg_version_t {
    enum ethcmd_subsystem_t sys;
    enum ethcmd_version_cmd_t cmd;
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[4];
} packed;

struct ethcmd_msg_fs20_t {
    enum ethcmd_subsystem_t sys;
    enum ethcmd_fs20_cmd_t cmd;
    uint16_t housecode;
    uint8_t address;
    uint8_t command;
    uint8_t reserved[2];
} packed;

struct ethcmd_msg_storage_t {
    enum ethcmd_subsystem_t sys;
    enum ethcmd_version_cmd_t cmd;
    uint8_t length;
    uint8_t reserved[5];
} packed;

#endif
