/*
 *
 * Copyright (c) 2013 by Daniel Lindner <daniel.lindner@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>

#include "core/debug.h"
#include "bsbport.h"
#include "bsbport_helper.h"
#include "bsbport_rx.h"

struct bsbport_buffer_msg bsbport_msg_buffer;

void
bsbport_rx_init(void)
{
	bsbport_msg_buffer.act=0;
	for(uint8_t i=0;i<BSBPORT_MESSAGE_BUFFER_LEN;i++)
	{
		bsbport_msg_buffer.msg[i].len=0;
		bsbport_msg_buffer.msg[i].value_temp=0;
		bsbport_msg_buffer.msg[i].value_FP1=0;
		bsbport_msg_buffer.msg[i].value_FP5=0;
		bsbport_msg_buffer.msg[i].value_raw=0;
		for(uint8_t j=0;j<BSBPORT_MESSAGE_MAX_LEN;j++)
		{
			bsbport_msg_buffer.msg[i].data[j]=0;
		}
	}
}

void
bsbport_rx_periodic(void)
{
	uint8_t i=0,read;
	uint8_t buffer[BSBPORT_MESSAGE_MAX_LEN];

	while (bsbport_recv_buffer.len > bsbport_recv_buffer.read)
//	&& bsbport_recv_buffer.len > 11)	// Minimal Message Size reached 
	{
		#ifdef DEBUG_BSBPORT_RX
			debug_printf("Start Read: %u bytes avail ",bsbport_recv_buffer.len);
		#endif 

		// Read serial data...
		read = bsbport_recv_buffer.data[bsbport_recv_buffer.read++];


		// ... until SOF detected (= 0xDC)
		if (read == 0xDC) 
		{
			i=0;
			// Restore otherwise dropped SOF indicator
			buffer[i++] = read;

			// read the rest of the message
			while (bsbport_recv_buffer.len > bsbport_recv_buffer.read)
//			&& bsbport_recv_buffer.len > 10)	// Minimal Message Size still reached 
			{
				read = bsbport_recv_buffer.data[bsbport_recv_buffer.read++];
				buffer[i++] = read;

				// Break if next byte signals next message, shouldn´t do that when reading partial messages because of endless loop
				if(0
				//((bsbport_recv_buffer.len > bsbport_recv_buffer.read) 
				//&& (bsbport_recv_buffer.data[bsbport_recv_buffer.read] == 0xDC))
				//	Break if we are at max message lenght	
				|| (i >= BSBPORT_MESSAGE_MAX_LEN)
				// Break if message seems to be completely received (i==msg.length)
				|| ((i > LEN)
				&& (i >= buffer[LEN])))
				{
					break;
				}
			}

			// Check if we have read the message completly.
			if (i == buffer[LEN]) 
			{
				// Seems to have received all data
				if (bsbport_crc(buffer, i) == 0)
				{ 
#ifdef DEBUG_BSBPORT_RX
					debug_printf("Valid: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],i);
#endif 
					bsbport_rx_ok++;
					/*	Store only Messages which contain information						*/
					/*	Store only Messages which are addressed to us						*/
					/*	Valid Message received	-> If type = Answer or Info Get Value		*/
					/*	Valid Message received	-> If dest = OwnAddress or Info Get Value		*/
					if (buffer[TYPE] == ANSWER
					|| buffer[TYPE] == INFO
					|| (buffer[DEST] & 0x0F) == BSBPORT_OWNADDRESS)
					{
						bsbport_store_msg(buffer,i);
					}

				}
				else	/*	CRC Error */
				{
#ifdef DEBUG_BSBPORT_RX
					debug_printf("CRC Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],i);
#endif
					bsbport_rx_crcerror++;
				}
			}
			// We are still missing some bytes but we still have space in databuffer -> Reset Readbytes and try again next time
			else if(i < buffer[LEN]
					&& i < BSBPORT_MESSAGE_MAX_LEN)
			{
#ifdef DEBUG_BSBPORT_RX
//				debug_printf("Partial: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],i);
#endif
				bsbport_recv_buffer.read=0; 
				break;
			}
			// Length error
			else if(i < buffer[LEN])
			{
#ifdef DEBUG_BSBPORT_RX
				debug_printf("LUV Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],i);
#endif
				bsbport_rx_lenghtunder++;
			}
			else if(i > buffer[LEN])
			{
#ifdef DEBUG_BSBPORT_RX
				debug_printf("LOV Error: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x Len:%d\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15],buffer[16],buffer[17],buffer[18],buffer[19],i);
#endif
				bsbport_rx_lenghtover++;
			}
		}
		else
		{
			bsbport_rx_dropped++;		
		}

		#ifdef DEBUG_BSBPORT_RX
			debug_printf("Delete Bytes: %u ",bsbport_recv_buffer.read);
		#endif 

		/* data we have read can be cleared from buffer */
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			bsbport_recv_buffer.len -= bsbport_recv_buffer.read;
			memmove(bsbport_recv_buffer.data,
					bsbport_recv_buffer.data + bsbport_recv_buffer.read,
					bsbport_recv_buffer.len);
			bsbport_recv_buffer.read = 0;
		}
	}
}

void bsbport_calc_value(struct bsbport_msg* msg)
{
// Nach Nachrichten Typ entscheiden
	if(msg->data[TYPE] == INFO)
	{
#ifdef DEBUG_BSBPORT_PARSE
		debug_printf("DATA: %02x %02x %02x Len:%d TYPE: %u ",msg->data[DATA],msg->data[DATA+1],msg->data[DATA+2],msg->data[LEN],msg->data[TYPE]);
#endif
		msg->value_raw = bsbport_ConvertToInt16(&msg->data[DATA]);	
		msg->value_temp = bsbport_ConvertToTemp(msg->value_raw);
		msg->value_FP1 = bsbport_ConvertToFP1(msg->value_raw);
		msg->value_FP5 = bsbport_ConvertToFP5(msg->value_raw);
	}
	else if(msg->data[TYPE] == SET)
	{
		msg->value_raw = bsbport_ConvertToInt16(&msg->data[DATA]);	
		msg->value_temp = bsbport_ConvertToTemp(msg->value_raw);
		msg->value_FP1 = bsbport_ConvertToFP1(msg->value_raw);
		msg->value_FP5 = bsbport_ConvertToFP5(msg->value_raw);
	}
	else if(msg->data[TYPE] == ACK)
	{
		//Ack has no value
	}
	else if(msg->data[TYPE] == QUERY)
	{
		//Query has no value
	}
	else if(msg->data[TYPE] == ANSWER)
	{
#ifdef DEBUG_BSBPORT_PARSE
	debug_printf("DATA: %02x %02x %02x Len:%d TYPE: %u ",msg->data[DATA],msg->data[DATA+1],msg->data[DATA+2],msg->data[LEN],msg->data[TYPE]);
#endif
		msg->value_raw = bsbport_ConvertToInt16(&msg->data[DATA+1]);	
		msg->value_temp = bsbport_ConvertToTemp(msg->value_raw);
		msg->value_FP1 = bsbport_ConvertToFP1(msg->value_raw);
		msg->value_FP5 = bsbport_ConvertToFP5(msg->value_raw);
	}
	else
	{
		//Unknown Type
#ifdef DEBUG_BSBPORT_PARSE
		debug_printf("Unknown Messagetype received: %02x \n",msg->data[TYPE]);
#endif
	}
#ifdef DEBUG_BSBPORT_PARSE
	debug_printf("Parsed as RAW %d FP1: %d FP5: %d TMP: %d",msg->value_raw,msg->value_FP1,msg->value_FP5,msg->value_temp);
#endif

}

void bsbport_store_msg(uint8_t* msg,uint8_t len)
{
	uint8_t saved=0;
#ifdef DEBUG_BSBPORT_RX
	debug_printf("Store MSG at POS: %d ",bsbport_msg_buffer.act);
#endif
		for(uint8_t i=0;i<BSBPORT_MESSAGE_BUFFER_LEN;i++)
		{
			if(bsbport_msg_buffer.msg[i].len != 0
				&& bsbport_msg_buffer.msg[i].data[SRC]==msg[SRC]
				&& bsbport_msg_buffer.msg[i].data[P1]==msg[P1]
				&& bsbport_msg_buffer.msg[i].data[P2]==msg[P2]
				&& bsbport_msg_buffer.msg[i].data[P3]==msg[P3]
				&& bsbport_msg_buffer.msg[i].data[P4]==msg[P4])
			{
				// Mark message valid 
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{
				memcpy(bsbport_msg_buffer.msg[i].data,msg,len);
				bsbport_msg_buffer.msg[i].len = len;
				bsbport_calc_value(&bsbport_msg_buffer.msg[i]);
				saved=1;
				}
			}
		}
		if(!saved)
		{			// Mark message valid 
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
			memcpy(bsbport_msg_buffer.msg[bsbport_msg_buffer.act].data,msg,len);
			bsbport_msg_buffer.msg[bsbport_msg_buffer.act].len = len;
			bsbport_calc_value(&bsbport_msg_buffer.msg[bsbport_msg_buffer.act++]);
			}
		}
	if(bsbport_msg_buffer.act >= BSBPORT_MESSAGE_BUFFER_LEN) bsbport_msg_buffer.act=0;
}

/*
  -- Ethersex META --
  header(protocols/bsbport/bsbport_rx.h)
  init(bsbport_rx_init)
  timer(5, bsbport_rx_periodic())
*/
