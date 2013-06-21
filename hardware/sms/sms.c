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
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#define USE_USART SMS_USE_USART
#define BAUD 9600

#include "config.h"
#include "core/debug.h"
#include "core/usart.h"
#include "core/periodic.h"
#include "sms.h"
#include "sms_ecmd.h"
#include "sms_encoding.h"

#define SMS_BUFFER 4
#define REINIT_MOBIL_TIMEOUT 50

#define SMS_TIMEOUT 100
#define RXBUFFER 200

/* buffer for mobile reply */
volatile unsigned char rx_buffer[RXBUFFER]; 
/* must parse mobile reply */
volatile uint8_t pdu_must_parse = 0;
/* protect mobile from multiple access */
volatile uint8_t global_mobil_access = 0;

/* index of rx_buffer */
static volatile uint8_t len = 0;
/* buffer for incoming data from mobile */
static volatile unsigned char endung[4];    
/* timeout for mobile replay */
static volatile uint8_t mobil_access_timeout = REINIT_MOBIL_TIMEOUT;
/* index of next free sms_buffer field */
static uint8_t put_index = 0;

/* buffer for messages, to be transmitted */ 
sms *sms_buffer[SMS_BUFFER];

/* states for sending a message */
enum {SEND_LEN, WAIT_FOR_START, SEND_SMS, WAIT_FOR_SMS_CONFIRM, 
        SMS_SEND_COMPLETE, SMS_SEND_FAILED};
volatile uint8_t state  = SEND_LEN;

/* We generate our own usart init module, for our usart port */
generate_usart_init()

static int noinline my_uart_put (const char, FILE *);

/* dummy FILE, required by fprintf */
FILE ausgabe = FDEV_SETUP_STREAM (my_uart_put, NULL, _FDEV_SETUP_WRITE);

ISR(usart(USART,_RX_vect)) {
    uint8_t data = usart(UDR);
    static int8_t i = -1;


    if ((data == '\n' && !strncmp((char *) endung, "OK\r",3))) {
        /* we received the whole answer from mobile */
        SMS_DEBUG("%3s\n", endung);
        SMS_DEBUG("unlock\r\n");
        global_mobil_access = 0;    
        i = -1;

        pdu_must_parse = 1;
        rx_buffer[len-1] = '\0';
        len = 0;
    }
    
    if (state >= WAIT_FOR_START)
        SMS_DEBUG("%c", data);

    if (i == 2)
        memmove((void *) endung, (void *) endung + 1, 3);
    else
        i ++;
    endung[i] = data;
    
    /* wait for reply "> ": mobil is ready to receive pdu */
    if (state == WAIT_FOR_START && data == ' ') {
        SMS_DEBUG("send sms\r\n");
        state = SEND_SMS;
        return;
    }
    /* wait for char '+': mobil has sent sms succuessfully */
    if (state == WAIT_FOR_SMS_CONFIRM && data == '+') {
        state = SMS_SEND_COMPLETE;
    }
    
    if (state == SEND_LEN && (len != sizeof(rx_buffer))) {
        rx_buffer[len++] = data;
    }   

    mobil_access_timeout = REINIT_MOBIL_TIMEOUT;
    return;
}



void sms_uart_init() 
{
    uint8_t i;
    
    usart_init();
    SMS_DEBUG("sms\r\n");
    /* disable command echo from mobil */
    fprintf(&ausgabe, "ate0\r\n");
    SMS_DEBUG("mobil echo off\r\n");
    global_mobil_access = 1;
    i = 255;
    while (global_mobil_access == 1 && i--)
        _delay_ms(4);
    if (!i)
        return;
    /* select mobil mem */
    fprintf(&ausgabe, "AT+CPMS=MT\r\n"); 
    SMS_DEBUG("mobil memory selected\r\n");
    global_mobil_access = 1;
    i = 255;
    while (global_mobil_access == 1 && i--)
        _delay_ms(4);
    if (!i)
        return;

    
    for (i = 0; i < SMS_BUFFER; i++)
        sms_buffer[i] = NULL;
}

static int
my_uart_put (const char d, FILE *stream)
{
    if (d == '\n')
        my_uart_put('\r', stream);
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = d;
    return 0;
}

void sms_periodic_timeout() 
{   
    if (global_mobil_access) {
        mobil_access_timeout --;

        if (!mobil_access_timeout) {
            global_mobil_access = 0;
        }
    }
}


void sms_transmit_handler() 
{
    static uint8_t current_sms;
    uint8_t i;
    uint8_t text_len;
    
    if (current_sms >= SMS_BUFFER)
        current_sms = 0;
    
    if (sms_buffer[current_sms] != NULL) {
        /*
            !pdu_must_parse: we can't start sending: rxbuffer 
                             will be overwritten by "> "
            state != SEND_LEN: we have already access to serial port,
                                we needn't check global_mobil_access
        */
        if (((state == SEND_LEN) && (!global_mobil_access)) ||
                (state != SEND_LEN)) {
            
            global_mobil_access = 1;
            /*  start sending a sms */
            switch (state) {

                case SEND_LEN:
                    global_mobil_access = 1;
                    /* we add all parts and divide it by 2 (>>1) */
                    fprintf(&ausgabe, "AT+CMGS=%u\r\n", 
                            ((sms_buffer[current_sms]->text_len * 2 +
                            strlen((char *) sms_buffer[current_sms]->rufnummer) +
                            10) >> 1));
                    SMS_DEBUG("AT+CMGS=%u\r\n", 
                            ((sms_buffer[current_sms]->text_len * 2 +
                            strlen((char *) sms_buffer[current_sms]->rufnummer) +
                            10) >> 1));
                    state = WAIT_FOR_START;
                    break;

                case WAIT_FOR_START:
                    if (mobil_access_timeout == 1) {
                        /* call error callback function -> send sms has failed */
                        state = SMS_SEND_FAILED;
                        SMS_DEBUG("WAIT_FOR_START\n");
                    }
                    break; 
                
                case SEND_SMS:
                    text_len = sms_buffer[current_sms]->text_len;
                    fprintf(&ausgabe, "001100");
                    fprintf(&ausgabe, "%s", sms_buffer[current_sms]->rufnummer);
                    fprintf(&ausgabe, "0000AA");
                    for (i = 0; i < text_len; i++) {
                        fprintf(&ausgabe, "%02X", 
                                            sms_buffer[current_sms]->text[i]);
                    }
                    fprintf(&ausgabe, "%c", 0x1a);

                    SMS_DEBUG( "\"001100");
                    SMS_DEBUG( "%s", sms_buffer[current_sms]->rufnummer);
                    SMS_DEBUG( "0000AA");
                    for (i = 0; i < text_len; i++) {
                        SMS_DEBUG( "%02X", sms_buffer[current_sms]->text[i]);
                    }
                    SMS_DEBUG( "\"\n");

                    state = WAIT_FOR_SMS_CONFIRM;
                    break;
                
                case WAIT_FOR_SMS_CONFIRM:
                    if (mobil_access_timeout == 1) {
                        /* call error callback function: send sms has failed */
                        state = SMS_SEND_FAILED;
                        SMS_DEBUG("WAIT_FOR_SMS_CONFIRM\n");
                    }
                    break;

                case SMS_SEND_FAILED:
                    if (sms_buffer[current_sms]->sms_send_err_calback 
                            !=  NULL) {
                        sms_buffer[current_sms]->sms_send_err_calback();
                    }
                    SMS_DEBUG("sending failed\r\n");
                    state = SEND_LEN;
                    break;

                case SMS_SEND_COMPLETE:
                    free(sms_buffer[current_sms]);
                    SMS_DEBUG("free done\r\n");
                    sms_buffer[current_sms] = NULL;
                    current_sms++;
                    state = SEND_LEN;
                    break;

                default:
                    break;
            }
        }
    }
    else {
        current_sms++;
    }
}


/* 
 * number must be of the form "49176123456",
 * where "49" is country specific for germany
 * the appropriate value for nr_is_encoded
 * should be "0" in most cases
 */

uint8_t sms_send(uint8_t *rufnummer, unsigned char *text,
                    void (*ptr)(), uint8_t nr_is_encoded) 
{
    uint8_t text_len = strlen((char *) text);

    SMS_DEBUG("sms_send: \"%s\",\r\n \"%s\"\r\n", rufnummer, text);

    if (text_len <= SMS_LEN) {
        /* search for a free buffer field */
        if (sms_buffer[put_index] == NULL) {
            sms_buffer[put_index] = malloc(sizeof(sms));
        }
        if (sms_buffer[put_index] == NULL) {
            SMS_DEBUG("malloc failed\r\n");
            /* send buffer full or malloc failed */
            return -1;
        }
        
        sms_buffer[put_index]->sms_send_err_calback = ptr;
        
        if (nr_is_encoded) {
            strcpy((char *) sms_buffer[put_index]->rufnummer, 
                                                            (char *) rufnummer);
        }
        else {
            sms_get_number(rufnummer, sms_buffer[put_index]->rufnummer);
        }
        sms_get_text(text, sms_buffer[put_index]->text, 
                                            &(sms_buffer[put_index]->text_len));
        
        put_index++;
        put_index = put_index % SMS_BUFFER;
    }
    return 0;
}

/*
  -- Ethersex META --
  header(hardware/sms/sms.h)
  init(sms_uart_init)
  timer(10, sms_periodic_timeout())
  mainloop(sms_transmit_handler)
*/

