/*
 * Copyright (c) 2009 by Stefan Krupop <mail@stefankrupop.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
 *
 * ----------------------------------------------------------------------------------|
 * |                                                                                 |
 * | Original Project : http://www.dmxcontrol.de/wiki/Art-Net-Node_f%C3%BCr_25_Euro  |
 * | Copied Version   : 17.01.2009                                                   |
 * |                                                                                 |
 * ----------------------------------------------------------------------------------|
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/bool.h"
#include "core/bit-macros.h"
#include "protocols/ws2801/ws2801.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "services/dmx-storage/dmx_storage.h"
#ifdef WS2801_SUPPORT

#define BAUD 250000
#define MAX_CHANNELS 			512

/* ----------------------------------------------------------------------------
 *global variables
 */

uint8_t ws2801_subNet = SUBNET_DEFAULT;
uint8_t ws2801_outputUniverse;
uint16_t ws2801_colortemp;
uint8_t ws2801_sendPollReplyOnChange = TRUE;
uip_ipaddr_t ws2801_pollReplyTarget;
uint32_t ws2801_pollReplyCounter = 0;
uint8_t ws2801_status = RC_POWER_OK;
char ws2801_shortName[18] = { '\0' };
char ws2801_longName[64] = { '\0' };

uint16_t ws2801_port = CONF_WS2801_PORT;
uint8_t ws2801_netConfig = NETCONFIG_DEFAULT;

volatile unsigned char  ws2801_dmxUniverse[MAX_CHANNELS];
volatile uint16_t ws2801_dmxChannels = 0;

const char ws2801_ID[8] PROGMEM = "Art-Net";

unsigned short curDmxCh = 0;


/* ----------------------------------------------------------------------------
 * initialization of network settings
 */
void
ws2801_netInit(void)
{
  ws2801_net_init();
}

/* ----------------------------------------------------------------------------
 * initialization of Art-Net
 */
void
ws2801_init(void)
{

  WS2801_DEBUG("Init\n");
  /* read Art-Net port */
  ws2801_port = CONF_WS2801_PORT;
  /* read netconfig */
  ws2801_netConfig = NETCONFIG_DEFAULT;

  /* read subnet */
  ws2801_subNet = SUBNET_DEFAULT;
  ws2801_outputUniverse = CONF_WS2801_UNIVERSE;
  ws2801_colortemp = 2500;
  strcpy_P(ws2801_shortName, PSTR("e6ArtNode"));
  strcpy_P(ws2801_longName, PSTR("e6ArtNode hostname: " CONF_HOSTNAME));

  uip_ipaddr_copy(ws2801_pollReplyTarget,all_ones_addr);

  /* net_init */
  ws2801_netInit();

  /* annouce that we are here  */
  WS2801_DEBUG("send PollReply\n");
  ws2801_sendPollReply();

  /* enable PollReply on changes */
  ws2801_sendPollReplyOnChange = TRUE;

  /* enable ws2801 */
  PIN_CLEAR(WS2801_CLOCK);
  PIN_CLEAR(WS2801_DATA);

  ws2801_setall(0);
  
  WS2801_DEBUG("init complete\n");
  return;
}

static void
ws2801_send(uint16_t len)
{
  uip_udp_conn_t ws2801_conn;
  uip_ipaddr_copy(ws2801_conn.ripaddr, ws2801_pollReplyTarget);
  ws2801_conn.rport = HTONS(ws2801_port);
  ws2801_conn.lport = HTONS(ws2801_port);
  uip_udp_conn = &ws2801_conn;

  uip_slen = len;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}


/* ----------------------------------------------------------------------------
 * send an ArtPollReply packet
 */
void
ws2801_sendPollReply(void)
{

  /* prepare ws2801 PollReply packet */
  struct ws2801_pollreply *msg =
    (struct ws2801_pollreply *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
  memset(msg, 0, sizeof(struct ws2801_pollreply));
  WS2801_DEBUG("PollReply allocated\n");
  strncpy_P((char *) msg->id, ws2801_ID, 8);

  msg->opcode = OP_POLLREPLY;

  msg->versionInfoH = (FIRMWARE_VERSION >> 8) & 0xFF;
  msg->versionInfo = FIRMWARE_VERSION & 0xFF;

  msg->subSwitchH = 0;
  msg->subSwitch = ws2801_subNet & 15;

  /* Report as 'AvrArtNode' http://www.dmxcontrol.de/wiki/Art-Net-Node_f%C3%BCr_25_Euro */
  msg->oem = 0x08b1;
  msg->ubeaVersion = 0;
  msg->status = 0;
  /* Report as Manufacturer "ESTA" http://tsp.plasa.org/tsp/working_groups/CP/mfctrIDs.php */
  msg->estaMan = 0xFFFF;
  strcpy(msg->shortName, ws2801_shortName);
  strcpy(msg->longName, ws2801_longName);
  sprintf(msg->nodeReport, "#%04X [%04u] e6ArtNode is ready", ws2801_status,
          (unsigned int) ws2801_pollReplyCounter);

  msg->numPortsH = 0;
  msg->numPorts = 1;

  msg->portTypes[0] = PORT_TYPE_DMX_OUTPUT;
  msg->goodInput[0] = (1 << 3);
  msg->goodOutput[0] = (1 << 1);
 
  msg->swout[0] = (ws2801_subNet & 15) * 16 | (ws2801_outputUniverse & 15);
  msg->style = STYLE_NODE;

  memcpy(msg->mac, uip_ethaddr.addr, 6);

  /* broadcast the packet */
  ws2801_send(sizeof(struct ws2801_pollreply));
}

int16_t
parse_cmd_ws2801_pollreply(int8_t * cmd, int8_t * output, uint16_t len)
{
  ws2801_sendPollReply();
  return ECMD_FINAL_OK;
}

void
processPollPacket(struct ws2801_poll *poll)
{
  if ((poll->talkToMe & 2) == 2)
    ws2801_sendPollReplyOnChange = TRUE;
  else
    ws2801_sendPollReplyOnChange = FALSE;
  if ((poll->talkToMe & 1) == 1)
	  uip_ipaddr_copy(ws2801_pollReplyTarget,uip_hostaddr);
  else
	  uip_ipaddr_copy(ws2801_pollReplyTarget,all_ones_addr);
  ws2801_sendPollReply();
}

/* ----------------------------------------------------------------------------
 * receive Art-Net packet
 */
void
ws2801_get(void)
{
  struct ws2801_header *header;
  static unsigned short curDmxCh = 0;

  header = (struct ws2801_header *) uip_appdata;

  /* check the id */
  if (strncmp_P((char *) header->id, ws2801_ID, 8))
  {
    WS2801_DEBUG("Wrong ArtNet header, discarded\r\n");
    ws2801_status = RC_PARSE_FAIL;
    return;
  }
  switch (header->opcode)
  {
    case OP_POLL:;
      struct ws2801_poll *poll;

      WS2801_DEBUG("Received ws2801 poll packet!\r\n");
      poll = (struct ws2801_poll *) uip_appdata;
      processPollPacket(poll);
      break;
    case OP_POLLREPLY:;
      WS2801_DEBUG("Received ws2801 poll reply packet!\r\n");
      break;
    case OP_OUTPUT:;
      struct ws2801_dmx *dmx;

      WS2801_DEBUG("Received ws2801 output packet!\r\n");
      dmx = (struct ws2801_dmx *) uip_appdata;

      if (dmx->universe == ((ws2801_subNet << 4) | ws2801_outputUniverse))
      {
	  ws2801_dmxChannels = (dmx->lengthHi << 8) | dmx->length;
          memcpy((unsigned char*)&ws2801_dmxUniverse[0], &(dmx->dataStart), ws2801_dmxChannels);
          if (ws2801_sendPollReplyOnChange == TRUE)
          {
            ws2801_pollReplyCounter++;
            ws2801_sendPollReply();
          }
   	/*
   	ws2801 Datenausgabe start
   	*/
        WS2801_DEBUG("write ws2801 channels \r\n");
	for(curDmxCh = 0; curDmxCh < ws2801_dmxChannels; curDmxCh++)
	{
		ws2801_writeByte(ws2801_dmxUniverse[curDmxCh]);
		WS2801_DEBUG("write ws2801 channel %x \r\n",ws2801_dmxUniverse[curDmxCh]);
	}
    
    	if (curDmxCh == ws2801_dmxChannels) {
    	ws2801_showPixel();
    	curDmxCh = 0;
	WS2801_DEBUG("write ws2801\r\n");
    	}
   	/*
   	ws2801 Datenausgabe ende
   	*/
      }
      break;
    case OP_ADDRESS:;
    case OP_IPPROG:;
      break;
    default:
      WS2801_DEBUG("Received an invalid ws2801 packet!\r\n");
      break;
  }
}

void ws2801_setall(unsigned char val)
{
        /*
   	ws2801 Datenausgabe start
   	*/
        WS2801_DEBUG("write ws2801 channels \r\n");
	for(curDmxCh = 0; curDmxCh < 510; curDmxCh++)
	{
		ws2801_writeByte(val);
		//WS2801_DEBUG("write ws2801 channel %x \r\n",ws2801_dmxUniverse[curDmxCh]);
	}
    
    	if (curDmxCh == ws2801_dmxChannels) {
    	ws2801_showPixel();
    	curDmxCh = 0;
	WS2801_DEBUG("write ws2801\r\n");
    	}
   	/*
   	ws2801 Datenausgabe ende
   	*/
}

void ws2801_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
        /*
   	ws2801 Datenausgabe start
   	*/
	uint16_t pix;
	for(pix = 0; pix < 171; pix++)
	{
	   if ((pix == n)||(n == 172)) {
		ws2801_writeByte(r);  //r
		ws2801_writeByte(g);  //g
		ws2801_writeByte(b);  //b
	   }
	   else {
		ws2801_writeByte(0);  //r
		ws2801_writeByte(0);  //g
		ws2801_writeByte(0);  //b
	   }
	}
    
    	ws2801_showPixel();

   	/*
   	ws2801 Datenausgabe ende
   	*/
}

void ws2801_setColorTemp(uint16_t k,uint8_t d)
{
    float Temperature,Red,Green,Blue;
    Temperature = k / 100;
    
    //Calculate Red:

    if (Temperature <= 66){
        Red = 255;
    } else {
        Red = Temperature - 60;
        Red = 329.698727446 * pow(Red,-0.1332047592);
        if (Red < 0) {
		Red = 0;
	}
        if (Red > 255) {
		Red = 255;
	}
    }
    
    //Calculate Green:

    if (Temperature <= 66){
        Green = Temperature;
        Green = 99.4708025861 * log(Green) - 161.1195681661;
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255){
		Green = 255;
    	}
    } else {
        Green = Temperature - 60;
        Green = 288.1221695283 * pow(Green,-0.0755148492);
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255) {
		Green = 255;
	}
    }
    
    //Calculate Blue:

    if (Temperature >= 66){
        Blue = 255;
    } else {
        if (Temperature <= 19) {
            Blue = 0;
        } else {
            Blue = Temperature - 10;
            Blue = 138.5177312231 * log(Blue) - 305.0447927307;
            if (Blue < 0) {
		 Blue = 0;
	    }
            if (Blue > 255) {
		Blue = 255;
	    }
        }

    }
    WS2801_DEBUG("kelvin: %d, R:%d G:%d B:%d \r\n",k, (uint8_t)Red,(uint8_t)Green,(uint8_t)Blue);
    uint8_t r,g,b;
	r=g=b=0;
	if ((d>0)&&(d<=100)) {
		r=(uint8_t)Red*d/100;
		g=(uint8_t)Green*d/100;
		b=(uint8_t)Blue*d/100;
	}

    WS2801_DEBUG("kelvin_dimmed: %d, Dim:%d R:%d G:%d B:%d \r\n",k,d,r,g,b);
    /*
    ws2801 Datenausgabe start
    */
	uint16_t pix;
	for(pix = 0; pix < 171; pix++)
	{
		ws2801_writeByte(r);  //r
		ws2801_writeByte(g);  //g
		ws2801_writeByte(b);  //b
	}
    
    	ws2801_showPixel();

     /*
     ws2801 Datenausgabe ende
     */
}

void ws2801_writeByte(unsigned char Send)
{
	register unsigned char BitCount = 8; // store variable BitCount in a cpu register
	do
	{
		PIN_CLEAR(WS2801_CLOCK);	// set clock LOW
		// send bit to ws2801. we do MSB first
		if (Send & 0x80) 
		{
    		PIN_SET(WS2801_DATA); // set output HIGH
		}
		else
		{
    		PIN_CLEAR(WS2801_DATA); // set output LOW
		}
		PIN_SET(WS2801_CLOCK); // set clock HIGH
		// next bit
		Send <<= 1;
	}while (--BitCount);
} // ws2801_writeByte

void ws2801_showPixel(void) {
    // when we're done we hold the clock pin low for a millisecond to latch it
    PIN_CLEAR(WS2801_CLOCK); // set clock LOW
    _delay_us(500); // wait for 500uS to display frame on ws2801
}
#endif /* WS2801_SUPPORT */

/*
   -- Ethersex META --
   header(protocols/ws2801/ws2801.h)
   net_init(ws2801_init)
   block(Miscelleanous)
 */
