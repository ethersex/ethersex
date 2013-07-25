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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include "core/usart.h"
#include "core/debug.h"
#include "sms_encoding.h"
#include "sms.h"
#include "sms_ecmd.h"
#include "protocols/ecmd/parser.h"
#include "protocols/ecmd/ecmd-base.h"

extern volatile uint8_t pdu_must_parse;
extern volatile unsigned char rx_buffer[]; 

#define DEBUG_ENABLE
#define DEBUG_REC
/* define max sms length */
//#define DATA_LEN ((160*7)/ 8) * 2 + 1
#define WRITE_BUFF SMS_LEN-1

static char write_buffer[WRITE_BUFF + 1];   
static uint8_t text[50];
char nummer[20];

static void read_sms(uint8_t *pdu) 
{
    uint16_t j = 2;
    uint8_t plain_len;
    uint8_t current_byte;
    uint8_t temp = 0;
    uint8_t i = 0, shift = 7;
    
    sscanf((char *) pdu, "%02hhX", &plain_len);
    while (i < plain_len) {
        sscanf((char *) pdu + j, "%02hhX", &current_byte);
        text[i++] = (temp | ((((uint8_t) current_byte) << (7 - shift)) & 0x7F));
        temp = current_byte >> shift;
        
        j += 2;
        shift --;
        if (shift == 0) {
            shift = 7;
            text[i++] = temp;
            temp = 0;
        }
    }
    text[i] = '\0';
    SMS_DEBUG("plain: %s, %d\r\n", text, i);
}

uint8_t pdu_parser(uint8_t *pdu) 
{
    uint8_t i = 3;
    uint8_t ret_val = 1;
    int16_t write_len = 0;
    uint16_t reply_len = 0;

    SMS_DEBUG("pdu_parser called: %d\n", pdu[i]);
    if (pdu[i++] == '+') {
        SMS_DEBUG("pdu_parser called: %d\n", pdu[i]);
        if (!strncmp((char *) pdu + i, "CMGL", 4)) {    
            SMS_DEBUG("messages will be received\r\n");
            uint8_t smsc_len, sender_len;
            if (reply_len == 0) {
                while (pdu[++i] != '\n');
                i ++;
                /* get length of smsc number */
                sscanf((char *) pdu + i, "%02hhX", &smsc_len);
                /* we ignore some flags "0000" */
                i += (smsc_len * 2) + 4;
                /* get data length */
                sscanf((char *) pdu + i, "%02hhX", &sender_len);
                /* get number of message sender */
                strncpy(nummer, (char *) pdu + i, sender_len + 5);
                nummer[sender_len + 5] = '\0';
                i = i + sender_len + 5 + 18;
                read_sms((uint8_t *) pdu + i);
            }
            
            SMS_DEBUG("ecmd: %s\n", text);
            do {
                if (reply_len < (int16_t)sizeof(write_buffer)) {
                    write_len = ecmd_parse_command((char *) text, 
                                              write_buffer + reply_len, 
                                              sizeof(write_buffer) - reply_len - 1);
                    SMS_DEBUG("ret_len ecmd parser: %d\r\n", write_len);
                    if (is_ECMD_AGAIN(write_len)) {
                        reply_len += -(10 + write_len);
                        *(write_buffer + reply_len) = ' ';
                        reply_len++;
                    } 
                    else if (write_len > 0) {
                        SMS_DEBUG("finished: \"%s\"\n", write_buffer);
                        ret_val = 0;
                        reply_len += write_len;
                        break;
                    }
                    SMS_DEBUG("%s\n", write_buffer);
                }
                else {
                    break;
                }
            } while (write_len <= 0);
           
            write_buffer[reply_len] = '\0';
            sms_send((uint8_t *) nummer, (uint8_t *) write_buffer, NULL, 1);
        }
    }
    return ret_val;
}

void sms_fetch() 
{
    static uint8_t fetch_sms;
    static uint8_t delete_sms = 0;
    
    fetch_sms ++;
    if (pdu_must_parse) {
        uint8_t ret;
        SMS_DEBUG("pdu_parser called with: %s\r\n", rx_buffer);
        ret = pdu_parser((uint8_t *) rx_buffer);

        if(ret == 0) {
            /* we want to delete the recently received message */
            delete_sms = 1;
        }
        pdu_must_parse = 0;
    }
    
    if (!global_mobil_access && delete_sms) {
        global_mobil_access = 1;
        delete_sms = 0;
        /* delete received and read message */
        fprintf(&ausgabe, "AT+CMGD=1\r\n");  
        SMS_DEBUG("delete sms\r\n");
    }

    if (!global_mobil_access && !pdu_must_parse && (!(fetch_sms % 50))) {
        global_mobil_access = 1;
        /* fetch received and not read messages */
        fprintf(&ausgabe, "AT+CMGL=0\r\n"); 
        SMS_DEBUG("fetch sms\n\n\n");
    }
}


/*
  -- Ethersex META --
  header(hardware/sms/sms_ecmd.h)
  timer(10, sms_fetch())
*/
