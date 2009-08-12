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


