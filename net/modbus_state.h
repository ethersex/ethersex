/*
 * Copyright (c) 2008 by Christian Dietrich
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
 */

#ifndef MODBUS_STATE_H
#define MODBUS_STATE_H

#include "modbus_net.h"

/* state */
struct modbus_connection_state_t {
    unsigned       must_send           :1;
    unsigned       waiting_for_answer  :1;
    unsigned       new_data            :1;
    uint16_t       transaction_id;
    uint8_t        len;
    uint8_t        data[MODBUS_BUFFER_LEN];
};

#endif
