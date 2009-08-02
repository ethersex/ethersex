/**********************************************************
* driver for MCA-25 camera
*
* @Author   : Simon Schulz [avr<AT>auctionant.de]
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but
*
* WITHOUT ANY WARRANTY;
*
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE. See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 51
* Franklin St, Fifth Floor, Boston, MA 02110, USA
*
* http://www.gnu.de/gpl-ger.html
***********************************************************/

#include <util/delay.h>
#include "mca25.h"
#include "protocols/uip/uip.h"

#define USE_USART MCA25_USE_USART
#define BAUD 9600
#include "core/usart.h"

generate_usart_init_interruptless();

#undef BAUD
#define BAUD 460800
#include "core/setbaud.h"

#define nop() __asm__ __volatile__ ("nop" ::)

unsigned char mca25_cam_status;

static void
mca25_set_460800baud (void)
{
	_delay_ms(5);

  usart(UBRR,H) = HI8(UBRRH_VALUE);
  usart(UBRR,L) = LO8(UBRRL_VALUE);

  /* Set or not set the 2x mode */
  USART_2X();
}

/******************************* CONSTANTS **********************************/
//some constants (command sequence)
PROGMEM char MCA25_START_JPG[] = {
0xF9,0x81,0xEF,0x3F,0x83,0x00,0x82,0x71,0x00,0x58,0x3C,0x6D,0x6F,0x6E,0x69,0x74,0x6F,0x72,0x69,0x6E,0x67,0x2D,0x63,0x6F,
0x6D,0x6D,0x61,0x6E,0x64,0x20,0x76,0x65,0x72,0x73,0x69,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x74,0x61,0x6B,0x65,0x2D,0x70,0x69,0x63,0x3D,0x22,0x4E,
0x4F,0x22,0x20,0x73,0x65,0x6E,0x64,0x2D,0x70,0x69,0x78,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x65,0x6C,0x2D,0x73,0x69,0x7A,0x65,0x3D,0x22,0x36,0x34,0x30,0x2A,0x34,0x38,0x30,0x22,0x20,0x7A,0x6F,
0x6F,0x6D,0x3D,0x22,0x31,0x30,0x22,0x2F,0x3E,0x42,0x00,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x21,0x78,0x2D,0x62,0x74,0x2F,0x69,0x6D,0x61,0x67,0x69,0x6E,0x67,0x2D,0x6D,0x6F,0x6E,0x69,0x74,0x6F,
0x72,0x69,0x6E,0x67,0x2D,0x69,0x6D,0x61,0x67,0x65,0x00,0x8C,0xF9,
0xF9,0x81,0xEF,0x0D,0x4C,0x00,0x06,0x06,0x01,0x80,0x4B,0xF9};


PROGMEM char MCA25_START_CAPTURING_1[] = {
0xF9,0x81,0xEF,0x3F,0x83,0x00,0x69,0x71,0x00,0x3F,0x3C,0x6D,0x6F,0x6E,0x69,0x74,0x6F,0x72,0x69,
0x6E,0x67,0x2D,0x63,0x6F,0x6D,0x6D,0x61,0x6E,0x64,0x20,0x76,0x65,0x72,0x73,0x69,0x8C,0xF9};

PROGMEM char MCA25_START_CAPTURING_2[] = {
0xF9,0x81,0xEF,0x3F,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x74,0x61,0x6B,0x65,0x2D,0x70,
0x69,0x63,0x3D,0x22,0x59,0x45,0x53,0x22,0x20,0x7A,0x6F,0x6F,0x6D,0x3D,0x22,0x31,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x30,0x22,0x2F,0x3E,0x42,0x00,0x21,0x78,0x2D,0x62,0x74,0x2F,0x69,0x6D,0x61,
0x67,0x69,0x6E,0x67,0x2D,0x6D,0x6F,0x6E,0x69,0x74,0x6F,0x72,0x69,0x6E,0x67,0x2D,0x8C,0xF9};

PROGMEM char MCA25_START_CAPTURING_3[] = {
0xF9,0x81,0xEF,0x19,0x69,0x6D,0x61,0x67,0x65,0x00,0x4C,0x00,0x06,0x06,0x01,0x80,0x50,0xF9};

PROGMEM char MCA25_CONFIG_640x480[] = {
0xF9,0x81,0xEF,0x3F,0x82,0x01,0x3B,0x01,0x00,0x03,0x49,0x01,0x35,0x3C,0x63,0x61,0x6D,0x65,
0x72,0x61,0x2D,0x73,0x65,0x74,0x74,0x69,0x6E,0x67,0x73,0x20,0x76,0x65,0x72,0x73,0x69,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x6F,0x6E,0x3D,0x22,0x31,0x2E,0x30,0x22,0x20,0x77,0x68,0x69,0x74,0x65,
0x2D,0x62,0x61,0x6C,0x61,0x6E,0x63,0x65,0x3D,0x22,0x4F,0x46,0x46,0x22,0x20,0x63,0x6F,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x6C,0x6F,0x72,0x2D,0x63,0x6F,0x6D,0x70,0x65,0x6E,0x73,0x61,0x74,0x69,
0x6F,0x6E,0x3D,0x22,0x31,0x33,0x22,0x20,0x66,0x75,0x6E,0x2D,0x6C,0x61,0x79,0x65,0x72,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x3D,0x22,0x30,0x22,0x3E,0x3C,0x6D,0x6F,0x6E,0x69,0x74,0x6F,0x72,0x69,
0x6E,0x67,0x2D,0x66,0x6F,0x72,0x6D,0x61,0x74,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x67,0x3D,0x22,0x45,0x42,0x4D,0x50,0x22,0x20,0x70,0x69,0x78,0x65,0x6C,
0x2D,0x73,0x69,0x7A,0x65,0x3D,0x22,0x38,0x30,0x2A,0x36,0x30,0x22,0x20,0x63,0x6F,0x6C,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x6F,0x72,0x2D,0x64,0x65,0x70,0x74,0x68,0x3D,0x22,0x38,0x22,0x2F,0x3E,
0x0D,0x0A,0x3C,0x74,0x68,0x75,0x6D,0x62,0x6E,0x61,0x69,0x6C,0x2D,0x66,0x6F,0x72,0x6D,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x61,0x74,0x20,0x65,0x6E,0x63,0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x45,
0x42,0x4D,0x50,0x22,0x20,0x70,0x69,0x78,0x65,0x6C,0x2D,0x73,0x69,0x7A,0x65,0x3D,0x22,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x31,0x30,0x31,0x2A,0x38,0x30,0x22,0x20,0x63,0x6F,0x6C,0x6F,0x72,0x2D,
0x64,0x65,0x70,0x74,0x68,0x3D,0x22,0x38,0x22,0x2F,0x3E,0x0D,0x0A,0x3C,0x6E,0x61,0x74,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x69,0x76,0x65,0x2D,0x66,0x6F,0x72,0x6D,0x61,0x74,0x20,0x65,0x6E,0x63,
0x6F,0x64,0x69,0x6E,0x67,0x3D,0x22,0x22,0x20,0x70,0x69,0x78,0x65,0x6C,0x2D,0x73,0x69,0x8C,0xF9,
0xF9,0x81,0xEF,0x3F,0x7A,0x65,0x3D,0x22,0x36,0x34,0x30,0x2A,0x34,0x38,0x30,0x22,0x2F,0x3E,
0x0D,0x0A,0x3C,0x2F,0x63,0x61,0x6D,0x65,0x72,0x61,0x2D,0x73,0x65,0x74,0x74,0x69,0x6E,0x8C,0xF9,
0xF9,0x81,0xEF,0x0B,0x67,0x73,0x3E,0x0D,0x0A,0xAF,0xF9};

/*
 typical usage:
 
 1) mca25_init(); 
    -> trigger a hardware reset and activates mux transfer

 2) mca25_configure(); 
    -> set up image format etc.

 3) mca25_start_image_grab();
    -> activate image grabbing, take a preview image

 4) mca25_grab_jpeg();
    -> activate jpg transfer

 5) mca25_grab_data();
    -> get x byte data

 6) while (){ mca25_send_data_ack(); mca25_grab_data(); ... }
    -> ack & grab loop

 => see mca25_copy_image_data_to_tcp_buffer() as an example ;)
 
*/

static void
mca25_uart_send (PGM_P ptr, uint16_t len)
{
  MCA25_DEBUG("mca25_uart_send ptr=%p, len=%d\n", ptr, len);

  for (; len; ptr ++, len --) {
    while (!(usart(UCSR,A) & _BV(usart(UDRE))));
		usart(UDR) = pgm_read_byte (ptr);
  }
}

#define MCA25_SEND(s)      mca25_uart_send(PSTR(s), sizeof(s)-1)
#define mca25_pgm_send(s)  mca25_uart_send(s, sizeof(s))

/*======================================================================
| copy the image data to the tcp data buffer, 
| buffer must be at least CAM_BUFFER_LEN byte long !
`======================================================================*/
unsigned char mca25_copy_image_data_to_tcp_buffer(char *buffer, int *bufferlen){
	unsigned int len = 0;
	unsigned char frametype = 0;

	//if we have had an error, we need to skip 
	//the remaining picture. cam has no ABORT cmd ?!?!
	if (mca25_cam_status == MCA25_SKIP_PICTURE){
			frametype = 0x48;
			
			//skip the current active picture...
			//dirty hack but seems like cam does
			//not have any abort commands :(
			while (frametype == 0x48){
				mca25_send_data_ack();
	
				mca25_grab_data(buffer, &len, &frametype); //grabs 250 byte data
			}	
			MCA25_SEND("\xF9\x01\xEF\x0B\xE3\x07\x23\x0C\x01\x79\xF9");
			
			mca25_cam_status = MCA25_FIRST_DATA;	
	}
		
	if (mca25_cam_status == MCA25_FIRST_DATA){
		//start grab here, this takes a 
		//long time...
		mca25_start_image_grab();

		//initialise jpg dump
		mca25_grab_jpeg();
		
		//first packet done
		mca25_cam_status = MCA25_NEXT_DATA;
	}else{ // if (mca25_cam_status == MCA25_NEXT_DATA){
		mca25_send_data_ack();
	}
	
	// we use the ethernet buffer for
	// storing the image data
	// --> make sure it is big enough ! (fixme)
	mca25_grab_data(buffer, &len, &frametype); 
	
	//sometimes the last packet seems to be empty
	//-> send this dummy data, it does not matter ...
	if (len == 0)
		len = CAM_BUFFER_LEN;
	
	// last picture is XX SH SL CC 00 
	// CC = 0x48 -> more data (?)
	//    = 0x49 -> last data
	if (frametype!=0x48){
		// this is important ! after the image grad
		// we need to do this! without this the camera
		// sometimes hangs while grabbing another image ... strange
		// reconfig mux (?)
		MCA25_SEND("\xF9\x01\xEF\x0B\xE3\x07\x23\x0C\x01\x79\xF9");
		
		return 0; // this is a smaller packet -> it was the last
	}else
		return 1; // this is a full packet -> there should be more (fixme)
}

/*======================================================================
| grab the next x byte data frame
| (cam must bei in jpg capture mode!)
`======================================================================*/
void mca25_grab_data(char *buffer, unsigned int *datalen, unsigned char *frametype){
	unsigned int j=0;
	unsigned char togo=31;
	unsigned char rx=0;
	unsigned char state=0;
	unsigned char firstframe = 1;

	*datalen = 0;
	
	// we start with len=0,
	// we extract the packetlength 
	// after the first packet and update len
	//
	// if we re in state12 -> continue, we do not have the full len yet
	while( *datalen==0 || (j<*datalen) || state>99 || state == 12 ){
		MCA25_STATUS_LED_ON();
		while (!(usart(UCSR,A) & _BV(usart(RXC))));
		rx = usart(UDR);
		MCA25_STATUS_LED_OFF();
		
		switch(state){
			case 0: 
				//no packet header rcv, wait for header:
				if (rx == 0xF9)
					state = 1;
				//else: do nothing
				break;
			case 1:
				//we got a F9
				if (rx == 0x83)
					state = 2; //we have a data packet
				else if (rx == 0xF9){
					state = 1; //this is the real start byte
					//MCA25_ERROR_LED_ON();
				}else{
					state = 0; // we missed something, try again.
					//MCA25_ERROR_LED_ON();
				}
				break;
			case 2:
				//now we expect EF
				if (rx == 0xEF){
					state = 3;	// packet ok
				}else{
					if (rx == 0xF9){
						//??? happens sometimes
						state = 1;
					}else{
						state = 0; // something went wrong -> retry
						//MCA25_ERROR_LED_ON();
					}
				}
				break;
			case 3: 	
				//next byte is frame len:
				togo = (rx-1)/2; //rx/2
				//printf("len=%d\n",togo);
				//if (togo != 31 && togo != 13) printf("len=%d\n",togo);
				if (firstframe==1)
					state = 10; //get frame info
				else
					state = 100; //grab data
				break;

			case 10:
				//90 01 00 48 00 xx
				//this is the first packet and we 
				//have not sampled anything
				//-> this is 0x90 -> ignore
				togo--;
				state = 11; 
				firstframe = 0;
				break;

			case 11:
				//this is the first packet and 
				//this byte is hi(length)
				*datalen = (unsigned int)(rx<<8);
				togo--;
				state = 12;
				break;

			case 12:
				//this is the first packet and 
				//this byte is lo(length)
				
				*datalen = (unsigned int)*datalen + (unsigned int)(rx) - 6; //substract the first
				                                                            //6byte frame info
				
				//make sure len is valid
				if (*datalen > CAM_BUFFER_LEN){
					*datalen = CAM_BUFFER_LEN;
				}
				
				togo--;
				state = 13;
				break;

			case 13:
				//this is the first packet and 
				//this byte is the frame type 
				*frametype = rx;
				togo--;
				state = 14;
				break;

			case 14:
				//this is the first packet and 
				//this byte is ??? -> ignore
				togo--;
				state = 15;
				break;

			case 15:
				//this is the first packet and 
				//this byte is ??? -> ignore
				togo--;
				state = 100; //now sample data
				break;
			
			case 100:
				//now sample data:
				if (j<CAM_BUFFER_LEN)
					buffer[j] = rx;
				else
					MCA25_ERROR_LED_ON(); //printf("ARGHHHHHHHHHHHHHHH\n");
				//printf("OUT[%03d] 0x%02x\n",j,rx);
				j++;
				
				togo--;
				if (togo == 0)
					state = 101;
				break;

			case 101:
				//data is there now we read checksum:
				//ignore CS...
				//printf("CSUM=%02x\n",rx);
				state = 102;
				break;
			case 102: 
				//we have frame end:
				if (rx != 0xF9){
						//MCA25_ERROR_LED_ON();
						//printf_P(PSTR("FRAME ERROR! @packet:%d: (0x%02x) len=%d\n"),j,rx,togo);
				}

				//printf("EOF=%d\n\n",rx);
				state = 0;
				break;
			default:
				MCA25_DEBUG("yeah stack problems. out of mem ? :-X\n");
		}
	}
}


/*======================================================================
| start a jpg grab
| (mca_25_start_image_grab() has to be called first)
`======================================================================*/
void mca25_grab_jpeg(){
	// send capture start cmd:
	mca25_pgm_send(MCA25_START_JPG);
  

	//send an ok for the power consumption message
	//printf_P(PSTR("\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9"));	  
}


/*======================================================================
| initialise the image grabbing
| it starts preview mode and "skips" a whole preview image
`======================================================================*/
void mca25_start_image_grab(){
	unsigned char state;
	unsigned char datapos;
	unsigned char buf[MCA25_COMM_BUFFER_LEN];
	
	//grab 6 preview pictures:
	for (char i=0; i<6; i++){
	
		/*while ( memcmp(buf,"\xF9\x83\xEF\x07\xA0\x00\x03",7) != 0)
			mca25_read_mux_packet(buf);
		*/
		
		// wait for go ?! fIXME
	 	// f9 83 f9 00 32 02  ???
		mca25_read_mux_packet(buf);
		
		// send capture start cmd:
		mca25_pgm_send(MCA25_START_CAPTURING_1);
		// this delay is neccessary !
		// without this we get a lot of
		// noise in the picture !
		_delay_ms(10);
		mca25_pgm_send(MCA25_START_CAPTURING_2);
		mca25_pgm_send(MCA25_START_CAPTURING_3);
		
		//send an ok for the power consumption message	
		MCA25_SEND("\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9");
	
		mca25_send_data_ack();
		
		state = 0;
		datapos = 0;
	
		// grab actual image and tell cam to keep it in RAM
		// we acknowledge the _preview_ image and download
		// the full size image later...
		while (state != 100){
			mca25_read_mux_packet(buf); //read MUX packet
			switch (state){
				case 0:
					// wait for first packet, decode if is last data
					// [F9 83 EF 3F 90 01 00 xx xx FD * ] F9 ]
					// xx xx = C3 00 -> first 256 byte
					// xx xx = 48 01 -> middle
					// xx xx = 49 01 -> last data!
				
#if CAM_BUFFER_LEN == 256
					if (memcmp_P(buf,PSTR("\xF9\x83\xEF\x3F\x90"),5) == 0)
#else
					// 512byte buf:
					// 90 02 00 C3 00 00 
					// 90 02 00 48 01 FD
					// A0 01 10 49 01 0D
					if (memcmp_P(buf,PSTR("\xF9\x83\xEF\x3F\x90\x02"),6) == 0)
#endif
					{
						if (buf[7] == 0xC3 && buf[8] == 0x00){
							//first frame:
							datapos = 1;
						}else if(buf[7] == 0x48 && buf[8] == 0x01){
							//middle
							datapos = 2;
						}else if(buf[7] == 0x49 && buf[8] == 0x01){
							//end:
							datapos = 3;
						}else if(buf[7] == 0x48 && buf[8] == 0x00){
							//end?
							datapos = 2;
						}else{
							//printf("buf7=%x, buf8=%x\n\n",buf[7],buf[8]);
						}
						state = 1;
						//last data -> send ack!	
						mca25_send_data_ack();
					}else if (memcmp_P(buf,PSTR("\xF9\x83\xEF\x3F\xA0"),5) == 0){
						// F9 83 EF 3F A0 00 4C 49 00 49 00 
						if(buf[7] == 0x49 && buf[8] == 0x00){
							//end when CAM_BUF_LEN = 256
							datapos = 3;
						}else if(buf[7] == 0x49 && buf[8] == 0x01){
							//end when CAM_BUF_LEN = 512
							datapos = 3;
						}else{
							//printf("buf7=%x, buf8=%x\n\n",buf[7],buf[8]);
						}
						state = 1;
						//last data -> send ack!	
						mca25_send_data_ack();
					}
					break;
	
				case 1:
					// wait for end of 256 Byte packet:
					// [F9 83 EF 11 ** ** ** ** ** ** ** ** 3F F9 ]
#if CAM_BUFFER_LEN == 256
					if ( (memcmp_P(buf,PSTR("\xF9\x83\xEF\x11"),4) == 0) ||
					     (memcmp_P(buf,PSTR("\xF9\x83\xEF\x1D"),4) == 0) ) {
#else
					if ( (memcmp_P(buf,PSTR("\xF9\x83\xEF\x21"),4) == 0) ||
				       (memcmp_P(buf,PSTR("\xF9\x83\xEF\x31"),4) == 0) ) {
#endif
						state =0;
	
						if (datapos == 3){
							//pic finished -> exit
							state = 100;
						}
					}
					break;
			}
		}
		//preview image #i has been grabbed.
	}	
}


/*======================================================================
| configure the camera
| (mca25_init() has to be called first !)
`======================================================================*/
void mca25_configure(){
	unsigned char state=0;
	unsigned char buf[MCA25_COMM_BUFFER_LEN];
	
	while (state != 100){
		mca25_read_mux_packet(buf); //read MUX packet
		switch (state){
			case 0:
				mca25_pgm_send(MCA25_CONFIG_640x480);
				state = 1;
				break;
			case 1:
				// wait for cam ACK:
				// [F9 83 EF 07 A0 00 03 C7 F9
				if (memcmp_P(buf,PSTR("\xF9\x83\xEF\x07\xA0\x00\x03\xC7\xF9"),9) == 0){
					// request camera info:
					// [F9 81 EF 2F 83 00 17 42 00 14 78 2D 62 74 2F 63 
					//  61 6D 65 72 61 2D 69 6E 66 6F 00 90 F9]
					MCA25_DEBUG("request camera info.\n");
					MCA25_SEND("\xF9\x81\xEF\x2F\x83\x00\x17\x42\x00\x14\x78\x2D\x62\x74\x2F\x63"
										 "\x61\x6D\x65\x72\x61\x2D\x69\x6E\x66\x6F\x00\x90\xF9");

					state = 2;
				}
				break;
			case 2:
				// ignore camera info ...
				// -> wait for last info packet:
				// [F9 83 EF 33 79 65 72 3D 22 31 30 22 2F 3E 3C 2F 
				//  63 61 6D 65 72 61 2D 69 6E 66 6F 3E 00 E4 F9]
				if (memcmp_P(buf,PSTR("\xF9\x83\xEF"), 3) == 0 && buf[3] != 0x3F) {
						MCA25_DEBUG("found end of camera info.\n");
						state = 100;
				}
				break;
		}
		
	}
}


/*======================================================================
| initialise the camera
| this has to be done at first, it activates the mux mode also
`======================================================================*/
unsigned char mca25_state;

int16_t mca25_init(void) {
	mca25_reset_cam();
	mca25_state = 1;
	return 0;
}

void mca25_process(void) {
	unsigned char buf[MCA25_COMM_BUFFER_LEN];

  if (!mca25_state || mca25_state == 100)
	  return;

	if (mca25_state < 10)
		mca25_read_at_command(buf); //read AT command
	else{
		mca25_read_mux_packet(buf); //read MUX packet
	}
	
	switch (mca25_state) {
		case 1:
			//we have to wait for AT&F:
			if (memcmp_P(buf,PSTR("AT&F"),4) == 0){
				//puts("AT&F");
				mca25_send_ok();
				mca25_state = 2;
			}
			break;
			
		case 2:
			//wait for AT+IPR
			if (memcmp_P(buf,PSTR("AT+IPR=?"),8) == 0){
				MCA25_SEND("+IPR: (),(1200,2400,4800,9600,19200,38400,57600,460800)\r\n\r\nOK\r\n");
				mca25_state = 3;
			}
			break;
		
		case 3:
			//wait for AT+IPR SET command
			if (memcmp_P(buf,PSTR("AT+IPR=460800"),13) == 0){
				mca25_send_ok();
				//set higher baudrate:
				mca25_set_460800baud();
				mca25_state = 5;
			}
			break;

		case 5:
			//wait for mux info req
			if (memcmp_P(buf,PSTR("AT+CMUX=?"),9) == 0){
				MCA25_SEND("\r\r\n+CMUX: (0),(0),(1-7),(31),(10),(3),(30),(10),(1-7)\r");
			}
			//wait for mux enable request
			else if (memcmp_P(buf,PSTR("AT+CMUX=0,0,7,31"),16) == 0){
				mca25_send_ok();
				mca25_state = 10;
			}
			break;

			case 10:
				// wait for mux ch0 request:
				// [F9 03 3F 01 1C F9]
				if (memcmp_P(buf,PSTR("\xF9\x03\x3F\x01\x1C\xF9"),6) == 0){
					// send mux ch0 ack/open packet:
					// [F9 03 73  01 D7 F9]
					MCA25_SEND("\xF9\x03\x73\x01\xD7\xF9");
					mca25_state = 11;
				}
				break;

			case 11:
				// wait for mux ch3 request:
				// [F9 23 3F 01 C9 F9]
				// after power-on cam sometimes seems to send [F9 23 F9] instead,
				// accept that one as well, since it seems to not hurt to go on.
				if (memcmp_P(buf,PSTR("\xF9\x23\x3F\x01\xC9\xF9"),6) == 0
						|| memcmp_P(buf,PSTR("\xF9\x23\xF9"),3) == 0) {
					// send mux ch3 ack/open packet:
					// [F9 23 73 01 02 F9]
					MCA25_SEND("\xF9\x23\x73\x01\x02\xF9");
					mca25_state = 12;
				}
				break;

			case 12:
				// wait for config mux ch0 request:
				// [F9 03 EF 09 E3 05 23 8D FB F9]
				if (memcmp_P(buf,PSTR("\xF9\x03\xEF\x09\xE3\x05\x23\x8D\xFB\xF9"),10) == 0){
					// send mux ch0 config ack/open packet:
					// [F9 01 EF 0B E3 07 23 0C 01 79 F9]
					MCA25_SEND("\xF9\x01\xEF\x0B\xE3\x07\x23\x0C\x01\x79\xF9");
					mca25_state = 13;
				}
				break;

			case 13:
				// wait for config mux ch3 request:
				// [F9 03 EF 09 E1 07 23 0C 01 FB F9]
				if (memcmp_P(buf,PSTR("\xF9\x03\xEF\x09\xE1\x07\x23\x0C\x01\xFB\xF9"),11) == 0){
					// send mux ch3 config ack/open packet:
					// [F9 01 EF 09 E1 05 23 8D 9A F9]
					MCA25_SEND("\xF9\x01\xEF\x09\xE1\x05\x23\x8D\x9A\xF9");
					mca25_state = 14;
				}
				break;

			case 14:
				// wait for AT*EACS.17.1.r:
				// [F9 23 EF 1B 41 54 2A 45 41 43 53 3D 31 37 2C 31 0D D1 F9]
				//if (memcmp(buf,"\xF9\x23\xEF\x1BAT*EACS=17,1\r\xD1\xF9",19) == 0)
					if (memcmp_P(buf,PSTR("\xF9\x23\xEF\x1B\x41\x54\x2A\x45\x41\x43"
												 "\x53\x3D\x31\x37\x2C\x31\x0D\xD1\xF9"),19) == 0){
					// send mux "\r\nOK\r\n" packet:
					// [F9 21 EF 0D 0D 0A 4F 4B 0D 0A 48  F9]
					MCA25_SEND("\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9");
					mca25_state = 15;
				}
				break;
				
			case 15:
				// wait for AT+CSCC=1,199\r5 peripheral AUTH req:
				// [F9 23 EF 1D 41 54 2B 43 53 43 43 3D 31 2C 31 39 39 0D 35 F9]
				if (memcmp_P(buf,PSTR("\xF9\x23\xEF\x1D\x41\x54\x2B\x43\x53\x43\x43"
											 "\x3D\x31\x2C\x31\x39\x39\x0D\x35\xF9"),20) == 0){
					// send response token:
					//DONT// [F9 21 EF 1D 41 54 2B 43 53 43 43 3D 31 2C 31 39 39 0D 54 F9]
					// [F9 21 EF 1B 0D 0A 2B 43 53 43 43 3A 20 45 33 0D 0A B0 F9 ]
					//printf("\xF9\x21\xEF\x1D\x41\x54\x2B\x43\x53\x43"
					//			 "\x43\x3D\x31\x2C\x31\x39\x39\x0D\x54\xF9"); 
					MCA25_SEND("\xF9\x21\xEF\x1B\x0D\x0A\x2B\x43\x53\x43"
								 "\x43\x3A\x20\x45\x33\x0D\x0A\xB0\xF9");
								 
					//append ok:
					MCA25_SEND("\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9");
					mca25_state = 16;
				}
				break;
				
			case 16:
				// wait for AT+CSCC=2,199.B9\r AUTH2 req:
				// AT+CSCC.2.199.B9.r|
				// [F9 23 EF 23 41 54 2B 43 53 43 43 3D 32 2C 31 39 39 2C 42 39 0D FB F9]
				if (memcmp_P(buf,PSTR("\xF9\x23\xEF\x23\x41\x54\x2B\x43\x53\x43\x43"
											 "\x3D\x32\x2C\x31\x39\x39\x2C\x42\x39\x0D\xFB\xF9"),23) == 0){
					// send response token: (OK)
					// [F9 21 EF 0D 0D 0A 4F 4B 0D 0A 48  F9]
					MCA25_SEND("\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9");
					
					// now request data mux channel (ch1):
					// [F9 81 3F 01 AB F9]
					MCA25_SEND("\xF9\x81\x3F\x01\xAB\xF9");
					mca25_state = 17;
				}
				break;
				
			case 17:
				// wait for mux ch1 ack:
				// [F9 81 73 01 60 F9]
				if (memcmp_P(buf,PSTR("\xF9\x81\x73\x01\x60\xF9"),6) == 0){
					// channel1 is now open!
					mca25_state = 18;
				}
				break;
				
			case 18:
				// wait for ch1 mux config:
				// [F9 03 EF 09 E3 05 83 8D FB F9]
				if (memcmp_P(buf,PSTR("\xF9\x03\xEF\x09\xE3\x05\x83\x8D\xFB\xF9"),10) == 0){
					// send config response:
					// [F9 01 EF 09 E1 05 83 8D 9A F9]
					MCA25_SEND("\xF9\x01\xEF\x09\xE1\x05\x83\x8D\x9A\xF9");
					
					// now configure cam mode:
					// [ F9 81 EF 37 80 00 1A 10 00 01 00 46 00 13 E3 3D
					//   95 45 83 74 4A D7 9E C5 C1 6B E3 1E DE 8E 61 82 F9 ]
					// F9 81 EF 37 			= mux header
					// 80 00 1A 10 00 	= ???
					// 01 00 						= (256-6)Byte data, then wait for ACK
					// 46  00 13 E3 3D 95 45 83 74 4A D7 9E C5 C1 6B E3 1E DE 8E 61 ???
					MCA25_SEND("\xF9\x81\xEF\x37\x80\x00\x1A\x10\x00"
					
					//buffsize
					"\x02\x00"
					
					"\x46\x00\x13\xE3\x3D\x95\x45\x83\x74\x4A\xD7"
					"\x9E\xC5\xC1\x6B\xE3\x1E\xDE\x8E\x61\x82\xF9"
					"\xF9\x21\xEF\x0D\x0D\x0A\x4F\x4B\x0D\x0A\x48\xF9");

					mca25_state = 19;
				}
				break;
				
			case 19:
				// cam should now accept our settings:
				// [F9 83 EF 3F A0 00 1F 10 00 20 00 CB 00 00 00 01 4A 00
				//  13 E3 3D 95 45 83 74 4A D7 9E C5 C1 6B E3 1E DE 8E ED F9
				if (memcmp_P(buf,PSTR("\xF9\x83\xEF\x3F\xA0\x00\x1F\x10\x00\x20\x00\xCB\x00"
											 "\x00\x00\x01\x4A\x00\x13\xE3\x3D\x95\x45\x83\x74\x4A"
											 "\xD7\x9E\xC5\xC1\x6B\xE3\x1E\xDE\x8E\xED\xF9"),37) == 0){
					mca25_configure();
					MCA25_DEBUG("mca25 cam configured!!!\n");
					mca25_state = 100; //-> exit init loop.
				}
				break;			
	}
}


/*======================================================================
| resets the camera (hw reset !)
`======================================================================*/
void mca25_reset_cam(){
	MCA25_RESET_LO();
  for(uint8_t i = 5; i; i --)
    _delay_ms(10);
  usart_init();
	MCA25_RESET_HI();
}



/************************** HELPER ***********************************/
void mca25_send_data_ack(){
	MCA25_SEND("\xF9\x81\xEF\x07\x83\x00\x03\xA6\xF9");
}

void mca25_send_ok(){
	MCA25_SEND("\r\r\nOK\r\n"); //puts adds a newline !!
}

void mca25_read_mux_packet(unsigned char *buffer){
	unsigned int cnt;
	for(cnt=0;cnt < MCA25_COMM_BUFFER_LEN - 1;cnt++){
		MCA25_STATUS_LED_ON();
		uint16_t timeout = 0;
		while (!(usart(UCSR,A) & _BV(usart(RXC))))
		  if(!--timeout) {
			  buffer[cnt] = 0;
				return;
			}
		buffer[cnt] = usart(UDR);
		MCA25_STATUS_LED_OFF();
		if (cnt>0 && buffer[cnt] == 0xF9){
			buffer[cnt+1] = '\0';
			break; //we have finished out read.
		}
	}
#if 0
  MCA25_DEBUG("read mux packet: ", buffer);
#ifdef DEBUG_MCA25
	for (uint16_t i = 0; i <= cnt; i ++) {
	  uint8_t j = buffer[i] >> 4;
		debug_putchar (j < 10 ? j + 48 : j + 55);
		j = buffer[i] & 15;
		debug_putchar (j < 10 ? j + 48 : j + 55);
	}
	debug_putchar(13);
	debug_putchar(10);
#endif
#endif

	return;
}

void mca25_read_at_command(unsigned char *buffer){
	unsigned int cnt;
	for(cnt=0;cnt<MCA25_COMM_BUFFER_LEN;cnt++){
		MCA25_STATUS_LED_ON();
		uint16_t timeout = 0;
		while (!(usart(UCSR,A) & _BV(usart(RXC))))
		  if(!--timeout) {
        buffer[cnt] = 0;
				return;
			}
		buffer[cnt] = usart(UDR);
		MCA25_STATUS_LED_OFF();
		if (buffer[cnt] == '\r' || buffer[cnt] == '\n'){
			buffer[cnt] = '\0';
			break; //we have finished out read.
		}
	}

  MCA25_DEBUG("read at '%s' (state %d).\n", buffer, mca25_state);
	return;
}

int16_t parse_cmd_mca25_reset(char *cmd, char *output, uint16_t len) __attribute__((alias("mca25_init")));


/*
  -- Ethersex META --
  header(hardware/camera/mca25.h)
  init(mca25_init)
  dnl init(mca25_configure)
	timer(1, mca25_process())
	ecmd_feature(mca25_reset, "mca25 reset")
*/
