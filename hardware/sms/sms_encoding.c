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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sms_encoding.h"

/* create number for pdu */ 
void sms_get_number(uint8_t *rufnummer, uint8_t *encoded_num) 
{
    uint8_t i = 0, j = 4;
    sprintf((char *) encoded_num, "%02X91", strlen((char *) rufnummer)); 
    
    while (rufnummer[i] != '\0') {
        encoded_num[j+1] = rufnummer[i];
        
        if (rufnummer[i+1] != '\0') {
            encoded_num[j] = rufnummer[i+1];
        }
        else {
            encoded_num[j] = 'F';
            j += 2;
            break;
        }
        i += 2; 
        j += 2;
    }
    encoded_num[j] = '\0';
}

/* 7-Bit encoding of payload */
void sms_get_text(uint8_t *sms_text, uint8_t *encoded_text, uint8_t *text_len) 
{
    uint8_t i = 0, j = 0, shift = 7;
    uint8_t data_len = strlen((char *) sms_text);
    encoded_text[i] = data_len;
    i++;

    while (j < data_len) {
        int temp = 0;
        encoded_text[i] = (sms_text[j] & 0x7F) >> (7 - shift);
        if (j < data_len - 1) {
            temp = (sms_text[j+1] & 0x7F) << shift;
        }
        encoded_text[i] = encoded_text[i] | temp;
        i++; j++;
        shift--;
        if (shift == 0) {
            shift = 7;
            j++;
        }
    }
    *text_len = i;
}


