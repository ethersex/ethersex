/*
 * Copyright (c) 2009 by Benjamin Sackenreuter <benson@zerties.org>
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

#ifndef _SMS_H
#define _SMS_H 

#define SMS_LEN 130

extern volatile uint8_t global_mobil_access;
extern FILE ausgabe;

typedef struct {
    unsigned char rufnummer[20];
    uint8_t text_len;
    unsigned char text[SMS_LEN];    
    void (*sms_send_err_calback)();
} sms;


uint8_t sms_send(uint8_t *rufnummer, unsigned char *text, void (*ptr)(), uint8_t nr_is_encoded);
void sms_transmit_handler(void);

void sms_uart_init(void);

void sms_periodic_timeout(void);

#ifdef DEBUG_SMS
# include "core/debug.h"
# define SMS_DEBUG(a...)  debug_printf(a)
//# define SMS_DEBUG(a...)  debug_printf("sms: " a)
#else
# define SMS_DEBUG(a...)
#endif

#endif
