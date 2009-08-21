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
#define USE_USART 0
#define BAUD 9600

#include "config.h"
#include "core/debug.h"
#include "core/usart.h"
#include "core/periodic.h"
#include "sms.h"
#include "sms_ecmd.h"
#include "sms_encoding.h"

#define SMS_BUFFER 5
#define SMS_RECV
#define DEBUG_ENABLE
#define DEBUG_COMM
#define DEBUG_LEN
#define REINIT_MOBIL_TIMEOUT 50

#define SMS_TIMEOUT 100
#define SMS_BUFFER 5
#define RXBUFFER 200

/* index of rx_buffer */
static volatile uint8_t len = 0;
/* buffer for incoming data from mobile */
static volatile unsigned char rx_buffer[RXBUFFER]; /* how much is really needed? -> one ecmds max size? */
static volatile unsigned char endung[6];	/* TODO: calc real size */
/* must parse mobile reply */
static volatile uint8_t pdu_must_parse = 0;
/* timeout for mobile replay */
static volatile uint8_t mobil_access_timeout = REINIT_MOBIL_TIMEOUT;
/* protect mobile from multiple access */
static volatile uint8_t global_mobil_access = 0;

static uint8_t put_index = 0;
FILE *ausgabe;

/* buffer for messages, to be transmitted */ 
sms *sms_buffer[SMS_BUFFER];

/* state for sending a message */
enum {SEND_LEN, WAIT_FOR_START, SEND_SMS, WAIT_FOR_SMS_CONFIRM, 
		SMS_SEND_COMPLETE, SMS_SEND_FAILED};
volatile uint8_t state  = SEND_LEN;

/* We generate our own usart init module, for our usart port */
generate_usart_init()

int noinline
my_uart_put (char d, FILE *stream);

/* interrupts seems to work perfect after some trouble ;-) ) */
SIGNAL(usart(USART,_RX_vect)) {
	uint8_t data = usart(UDR);
	static int8_t i = -1;


	if ((data == '\n' && !strncmp((char *) endung, "OK\r",3))) {
		/* we received the whole answer from mobile */
		debug_printf("%3s\n", endung);
		debug_printf("unlock\r\n");
		debug_printf("data %d\r\n", data);
		global_mobil_access = 0;	
		i = -1;
		pdu_must_parse = 1;
		rx_buffer[len-1] = '\0';
		len = 0;
	}
	
	#ifdef DEBUG_ENABLE
	if (state >= WAIT_FOR_START)
		debug_printf("%c", data);
	#endif

	if (i == 2)
		memmove((void *) endung, (void *) endung + 1, 3);
	else
		i ++;
	endung[i] = data;
  	
	/* wait for reply "> ": mobil is ready to receive pdu */
	if (state == WAIT_FOR_START && data == ' ') {
		debug_printf("send sms\r\n");
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
	/* open stream to mobil */
    ausgabe = fdevopen(my_uart_put, NULL);	
	debug_printf("sms\r\n");
	/* disable command echo from mobil */
	fprintf(ausgabe, "ate0\r\n");
	global_mobil_access = 1;
	i = 255;
	while (global_mobil_access == 1 && i--)
		_delay_ms(4);
	if (!i)
		return;
#ifdef DEBUG_ENABLE
	debug_printf("mobil echo off\r\n");
#endif
	fprintf(ausgabe, "AT+CPMS=MT\r\n");	
	global_mobil_access = 1;
	i = 255;
	while (global_mobil_access == 1 && i--)
		_delay_ms(4);
	if (!i)
		return;

#ifdef DEBUG_ENABLE
	debug_printf("mem selected\r\n");
#endif

#ifdef DEBUG_ENABLE
	debug_printf("mobil memory selected\r\n");
#endif
	
	for (i = 0; i < SMS_BUFFER; i++)
		sms_buffer[i] = NULL;
}

int noinline
my_uart_put (char d, FILE *stream)
{
    if (d == '\n')
        debug_uart_put('\r', stream);
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
    usart(UDR) = d;
    return 0;
}

void sms_periodic_timeout() 
{
	static uint8_t fetch_sms;
	static uint8_t delete_sms = 0;
	
	fetch_sms ++;
	if (pdu_must_parse) {
		uint8_t ret;
		#ifdef DEBUG_ENABLE
			debug_printf("pdu_parser called with: %s\r\n", rx_buffer);
		#endif
		ret = pdu_parser((uint8_t *) rx_buffer);

		if(ret == 0) {
			// we want to delete the recently received message 
			delete_sms = 1;
		}
		pdu_must_parse = 0;
	}
	
	if (!global_mobil_access && delete_sms) {
		global_mobil_access = 1;
		delete_sms = 0;
		/* delete received and read message */
		fprintf(ausgabe, "AT+CMGD=1\r\n");	
	#ifdef DEBUG_ENABLE
		debug_printf("delete sms\r\n");
	#endif
	}

	if (!global_mobil_access && !pdu_must_parse && (!(fetch_sms % 50))) {
		global_mobil_access = 1;
		/* fetch received and not read messages */
		fprintf(ausgabe, "AT+CMGL=0\r\n"); 
	#ifdef DEBUG_ENABLE
		debug_printf("fetch sms\n\n\n");
	#endif
	}
	
	if (global_mobil_access) {
		mobil_access_timeout --;

		if (!mobil_access_timeout) {
			/* enable rx-interrupt */
			global_mobil_access = 0;
			mobil_access_timeout = REINIT_MOBIL_TIMEOUT;
			usart(UCSR,B) |= _BV(usart(RXEN));
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
		if (((state == SEND_LEN) && (!global_mobil_access) 
			&& (!pdu_must_parse)) || (state != SEND_LEN)) {

			global_mobil_access = 1;
			/*  start sending a sms */
			switch (state) {

				case SEND_LEN:
					global_mobil_access = 1;
					/* we add all parts and divide it by 2 (>>1) */
					fprintf(ausgabe, "AT+CMGS=%u\r\n", 
							((sms_buffer[current_sms]->text_len * 2 +
							strlen((char *) sms_buffer[current_sms]->rufnummer) +
							10) >> 1));
					#ifdef DEBUG_ENABLE 
					debug_printf("AT+CMGS=%u\r\n", 
							((sms_buffer[current_sms]->text_len * 2 +
							strlen((char *) sms_buffer[current_sms]->rufnummer) +
							10) >> 1));
					#endif
					state = WAIT_FOR_START;
				#ifdef DEBUG_ENABLE
					debug_printf("sending started at buff_index %d\r\n", current_sms);
				#endif
					break;

				case WAIT_FOR_START:
					if (mobil_access_timeout == 1) {
						/* call error callback function -> send sms has failed */
						state = SMS_SEND_FAILED;
					}
					break; 
				
				case SEND_SMS:
					text_len = sms_buffer[current_sms]->text_len;
					fprintf(ausgabe, "001100");
					fprintf(ausgabe, "%s", sms_buffer[current_sms]->rufnummer);
					fprintf(ausgabe, "0000AA");
					for (i = 0; i < text_len; i++) {
						fprintf(ausgabe, "%02X", 
											sms_buffer[current_sms]->text[i]);
					}
					fprintf(ausgabe, "%c", 0x1a);
				#ifdef DEBUG_ENABLE	
					debug_printf( "001100");
					debug_printf( "%s", sms_buffer[current_sms]->rufnummer);
					debug_printf( "0000AA");
					for (i = 0; i < text_len; i++) {
						debug_printf( "%02X", 
											sms_buffer[current_sms]->text[i]);
					}
					debug_printf( "%c\n");
				#endif


					state = WAIT_FOR_SMS_CONFIRM;
					break;
				
				case WAIT_FOR_SMS_CONFIRM:
					if (mobil_access_timeout == 1) {
						/* call error callback function: send sms has failed */
						state = SMS_SEND_FAILED;
					}
					break;

				case SMS_SEND_FAILED:
					if (sms_buffer[current_sms]->sms_send_err_calback 
							!= 	NULL) {
						sms_buffer[current_sms]->sms_send_err_calback();
					}
					debug_printf("sending failed\r\n");
					state = SMS_SEND_COMPLETE;
					break;

				case SMS_SEND_COMPLETE:
					free(sms_buffer[current_sms]);
					debug_printf("free done\r\n");
					sms_buffer[current_sms] = NULL;
					current_sms++;
					state = SEND_LEN;
					mobil_access_timeout = REINIT_MOBIL_TIMEOUT;
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


uint8_t sms_send(uint8_t *rufnummer, unsigned char *text,
					void (*ptr)(), uint8_t nr_is_encoded) 
{
	uint8_t text_len = strlen((char *) text);

	debug_printf("sms_send: \"%s\",\r\n \"%s\"\r\n", rufnummer, text);

	if (text_len < 120) {
		/* search for a free buffer field */
		if (sms_buffer[put_index] == NULL) {
			sms_buffer[put_index] = malloc(sizeof(sms));
		}
		if (sms_buffer[put_index] == NULL) {
		#ifdef DEBUG_ENABLE
			debug_printf("malloc failed\r\n");
		#endif
			/* send buffer full or malloc failed */
			return -1;
		}
		
		sms_buffer[put_index]->sms_send_err_calback = ptr;
		
		if (nr_is_encoded) {
			strcpy((char *) sms_buffer[put_index]->rufnummer, (char *) rufnummer);
		}
		else {
			sms_get_number(rufnummer, sms_buffer[put_index]->rufnummer);
		}
		sms_get_text(text, sms_buffer[put_index]->text, &(sms_buffer[put_index]->text_len));
		
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

