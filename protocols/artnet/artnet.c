/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * Author:         Stefan Krupop <mail@stefankrupop.de>
 *                 Dirk Pannenbecker <dp@sd-gp.de>
 *		   Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * taken from:
 *   http://www.dmxcontroler.de/wiki/Art-Net-Node_für_25_Euro
 *    Copyright:      Stefan Krupop  mailto: mail@stefankrupop.de
 *    Author:         Stefan Krupop
 *    Remarks:        
 *    known Problems: none
 *    Version:        17.01.2009
 *    Description:    Implementation des ArtNet-Protokolls für DMX-Übertragung über Ethernet
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
#include "core/bool.h"
#include "core/bit-macros.h"
#include "protocols/artnet/artnet.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "services/dmx-storage/dmx_storage.h"
#ifdef ARTNET_SUPPORT


#include <stdio.h>
#include <avr/interrupt.h>


/* ----------------------------------------------------------------------------
 * op-codes
 */
#define OP_POLL			0x2000
#define OP_POLLREPLY		0x2100
#define OP_OUTPUT		0x5000
#define OP_ADDRESS		0x6000
#define OP_IPPROG		0xf800
#define OP_IPPROGREPLY		0xf900

/* ----------------------------------------------------------------------------
 * status
 */
#define RC_POWER_OK		0x01
#define RC_PARSE_FAIL		0x04
#define RC_SH_NAME_OK		0x06
#define RC_LO_NAME_OK		0x07

/* ----------------------------------------------------------------------------
 * default values
 */
#define SUBNET_DEFAULT		0
#define INUNIVERSE_DEFAULT	CONF_ARTNET_INUNIVERSE
#define OUTUNIVERSE_DEFAULT	CONF_ARTNET_OUTUNIVERSE
#define NETCONFIG_DEFAULT	1

/* ----------------------------------------------------------------------------
 * other defines
 */
#define MAX_NUM_PORTS		CONF_ARTNET_MAX_PORTS
#define SHORT_NAME_LENGTH	18
#define LONG_NAME_LENGTH	64
#define PORT_NAME_LENGTH	32
#define MAX_DATA_LENGTH		CONF_ARTNET_MAX_DATA_LENGTH

#define PROTOCOL_VERSION 	14 	/* DMX-Hub protocol version. */
#define FIRMWARE_VERSION 	0x0100	/* DMX-Hub firmware version. */
#define OEM_ID 			0xff00  /* OEM Code, just testcode as yet. */
#define STYLE_NODE 		0    	/* Responder is a Node (DMX <-> Ethernet Device) */

#define PORT_TYPE_DMX_OUTPUT	0x80
#define PORT_TYPE_DMX_INPUT 	0x40

#define MAX_CHANNELS 		CONF_ARTNET_MAX_CHANNELS
#define IBG   			10	/* interbyte gap [us] */

#define REFRESH_INTERVAL	4	/* [s] */

/* ----------------------------------------------------------------------------
 * packet formats
 */
struct artnet_packet_addr {
	uint8_t  ip[4];
	uint16_t port;
};

struct artnet_header {
	uint8_t  id[8];
	uint16_t opcode;
};

struct artnet_poll {
	uint8_t  id[8];
	uint16_t opcode;
	uint8_t  versionH;
	uint8_t  version;
	uint8_t  talkToMe;
	uint8_t  pad;
};

struct artnet_pollreply {
	char  id[8];
	uint16_t opcode;
	struct artnet_packet_addr addr;
	uint8_t  versionInfoH;
	uint8_t  versionInfo;
	uint8_t  subSwitchH;
	uint8_t  subSwitch;
	uint16_t oem;
	uint8_t  ubeaVersion;
	uint8_t  status;
	uint16_t estaMan;
	char          shortName[SHORT_NAME_LENGTH];
	char          longName[LONG_NAME_LENGTH];
	char          nodeReport[LONG_NAME_LENGTH];
	uint8_t  numPortsH;
	uint8_t  numPorts;
	uint8_t  portTypes[MAX_NUM_PORTS];
	uint8_t  goodInput[MAX_NUM_PORTS];
	uint8_t  goodOutput[MAX_NUM_PORTS];
	uint8_t  swin[MAX_NUM_PORTS];
	uint8_t  swout[MAX_NUM_PORTS];
	uint8_t  swVideo;
	uint8_t  swMacro;
	uint8_t  swRemote;
	uint8_t  spare1;
	uint8_t  spare2;
	uint8_t  spare3;
	uint8_t  style;
	uint8_t  mac[6];
	uint8_t  filler[32];
};

struct artnet_ipprog {
	char  id[8];
	uint16_t opcode;
	uint8_t  versionH;
	uint8_t  version;
	uint8_t  filler1;
	uint8_t  filler2;
	uint8_t  command;
	uint8_t  filler3;
	uint8_t  progIp[4];
	uint8_t  progSm[4];
	uint8_t  progPort[2];
	uint8_t  spare[8];
};

struct artnet_ipprogreply {
	char  id[8];
	uint16_t opcode;
	uint8_t  versionH;
	uint8_t  version;
	uint8_t  filler1;
	uint8_t  filler2;
	uint8_t  filler3;
	uint8_t  filler4;
	uint8_t  progIp[4];
	uint8_t  progSm[4];
	uint8_t  progPort[2];
	uint8_t  spare[8];
};

struct artnet_address {
	char  id[8];
	uint16_t opcode;
	uint8_t  versionH;
	uint8_t  version;
	uint8_t  filler1;
	uint8_t  filler2;
	int8_t          shortName[SHORT_NAME_LENGTH];
	int8_t          longName[LONG_NAME_LENGTH];
	uint8_t  swin[MAX_NUM_PORTS];
	uint8_t  swout[MAX_NUM_PORTS];
	uint8_t  subSwitch;
	uint8_t  swVideo;
	uint8_t  command;
};

struct artnet_dmx {
	uint8_t  id[8];
	uint16_t opcode;
	uint8_t  versionH;
	uint8_t  version;
	uint8_t  sequence;
	uint8_t  physical;
	uint16_t universe;
	uint8_t  lengthHi;
	uint8_t  length;
	uint8_t  dataStart;
};

/* ----------------------------------------------------------------------------
 *global variables
 */
enum {BREAK, STARTB, DATA, STOPPED};

uint8_t  artnet_subNet = SUBNET_DEFAULT;
uint8_t  artnet_outputUniverse1 = OUTUNIVERSE_DEFAULT;
uint8_t  artnet_inputUniverse1 = INUNIVERSE_DEFAULT;
uint8_t  artnet_sendPollReplyOnChange = TRUE;
uint64_t  artnet_pollReplyTarget = (uint64_t)0xffffffff;
uint32_t   artnet_pollReplyCounter = 0;
uint8_t  artnet_status = RC_POWER_OK;
char          artnet_shortName[18];
char          artnet_longName[64];
uint16_t artnet_port = CONF_ARTNET_PORT;
uint8_t  artnet_netConfig = NETCONFIG_DEFAULT;

volatile uint16_t artnet_dmxChannels = 0;
volatile uint8_t  artnet_dmxTransmitting = FALSE;
volatile uint8_t  artnet_dmxInChanged = FALSE;
volatile uint8_t  artnet_dmxInComplete = FALSE;
int8_t   artnet_conn_id = 0;
uint8_t  artnet_connected = 0;
uint8_t  artnet_dmxDirection = 0;
uint8_t  artnet_dmxRefreshTimer = REFRESH_INTERVAL;

/* ----------------------------------------------------------------------------
 * initialization of network settings
 */
	void
artnet_netInit(void)
{
	artnet_net_init();
}

/* ----------------------------------------------------------------------------
 * initialization of Art-Net
 */
void artnet_init(void) {

	ARTNET_DEBUG("Init\n");
	artnet_conn_id = dmx_storage_connect(INUNIVERSE_DEFAULT);
	if(artnet_conn_id != -1)
	{
		artnet_connected = TRUE;
		ARTNET_DEBUG("Connection to dmx-storage established! id:%d\r\n",artnet_conn_id);
	}
	else
	{
		artnet_connected = FALSE;
		ARTNET_DEBUG("Connection to dmx-storage couldn't be established!\r\n");
	}
	/* read Art-Net port */
	artnet_port = CONF_ARTNET_PORT;

	/* read netconfig */
	artnet_netConfig = NETCONFIG_DEFAULT;

	/* read subnet */
	artnet_subNet = SUBNET_DEFAULT;

	artnet_inputUniverse1 = INUNIVERSE_DEFAULT;

	artnet_outputUniverse1 = OUTUNIVERSE_DEFAULT;

	for (uint8_t i = 0; i < SHORT_NAME_LENGTH; i++) {
		artnet_shortName[i] = 0;
	}
	strcpy_P(artnet_shortName, PSTR("e6ArtNode"));
	artnet_shortName[SHORT_NAME_LENGTH - 1] = 0;

	/* read long name */
	for (uint8_t i = 0; i < LONG_NAME_LENGTH; i++) {
		artnet_longName[i] = 0;
	}
	strcpy_P(artnet_longName, PSTR("e6 artnet node hostname: " CONF_HOSTNAME));

	artnet_longName[LONG_NAME_LENGTH - 1] = 0;

	artnet_netInit();

	/* annouce that we are here  */
	ARTNET_DEBUG("send PollReply\n");
	artnet_sendPollReply();

	/* enable PollReply on changes */
	artnet_sendPollReplyOnChange = TRUE;

	ARTNET_DEBUG("init complete\n");
	return;
}

static void artnet_send (uint16_t len)
{
	uip_udp_conn_t artnet_conn;
	artnet_conn.ripaddr[0] = uip_hostaddr[0] | ~uip_netmask[0];
	artnet_conn.ripaddr[1] = uip_hostaddr[1] | ~uip_netmask[1];
	artnet_conn.rport = HTONS(artnet_port);
	artnet_conn.lport = HTONS(artnet_port);

	uip_udp_conn = &artnet_conn;

	uip_slen = len;
	uip_process (UIP_UDP_SEND_CONN);
	router_output ();

	uip_slen = 0;
}


/* ----------------------------------------------------------------------------
 * send an ArtPollReply packet
 */
void artnet_sendPollReply(void) {

	/* prepare artnet PollReply packet */
	struct artnet_pollreply *msg = 
		(struct artnet_pollreply *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
	memset(msg, 0, sizeof(struct artnet_pollreply));
	ARTNET_DEBUG("PollReply allocated\n");
	msg->id[0] = 'A';
	msg->id[1] = 'r';
	msg->id[2] = 't';
	msg->id[3] = '-';
	msg->id[4] = 'N';
	msg->id[5] = 'e';
	msg->id[6] = 't';

	msg->opcode = OP_POLLREPLY;

	msg->versionInfoH = (FIRMWARE_VERSION >> 8) & 0xFF;
	msg->versionInfo = FIRMWARE_VERSION & 0xFF;

	msg->subSwitchH = 0;
	msg->subSwitch = artnet_subNet & 15;

	msg->oem = (uint16_t)OEM_ID;
	msg->ubeaVersion = 0;
	msg->status = 0;
	msg->estaMan = 'D' * 256 + 'P';
	strcpy(msg->shortName, artnet_shortName);
	strcpy(msg->longName, artnet_longName);
	sprintf(msg->nodeReport, "#%04X [%04u] AvrArtNode is ready", artnet_status, (unsigned int)artnet_pollReplyCounter);

	msg->numPortsH = 0;
	msg->numPorts = 1;

	if (artnet_dmxDirection == 1) {
		msg->portTypes[0] = PORT_TYPE_DMX_INPUT;
	} else {
		msg->portTypes[0] = PORT_TYPE_DMX_OUTPUT;
	}

	if (artnet_dmxDirection != 1) {
		msg->goodInput[0] = (1 << 3);
	} else {
		if (artnet_dmxChannels > 0) {
			msg->goodInput[0] |= (1 << 7);
		}
	}

	msg->goodOutput[0] = (1 << 1);
	if (artnet_dmxTransmitting == TRUE) {
		msg->goodOutput[0] |= (1 << 7);
	}

	msg->swin[0] = (artnet_subNet & 15) * 16 | (artnet_inputUniverse1 & 15);
	msg->swout[0] = (artnet_subNet & 15) * 16 | (artnet_outputUniverse1 & 15);

	msg->style = STYLE_NODE;

	memcpy (msg->mac, uip_ethaddr.addr, 6);

	/* broadcast the packet */
	artnet_send(sizeof(struct artnet_pollreply));
}                                                                                      
/* ----------------------------------------------------------------------------
 * send an ArtDmx packet
 */
void artnet_sendDmxPacket(void) {
	static unsigned char sequence = 1;
	/* prepare artnet Dmx packet */
	struct artnet_dmx *msg = (struct artnet_dmx *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
	//struct artnet_dmx *msg = uip_appdata;
	memset(msg, 0, sizeof(struct artnet_dmx));

	msg->id[0] = 'A';
	msg->id[1] = 'r';
	msg->id[2] = 't';
	msg->id[3] = '-';
	msg->id[4] = 'N';
	msg->id[5] = 'e';
	msg->id[6] = 't';
	msg->id[7] =  0 ;
	msg->opcode = OP_OUTPUT;

	msg->versionH = 0;
	msg->version = PROTOCOL_VERSION;

	msg->sequence = sequence++;
	if (sequence == 0) {
		sequence = 1;
	}

	msg->physical = 1;
	msg->universe = ((artnet_subNet << 4) | artnet_inputUniverse1);
	msg->lengthHi = HI8(DMX_STORAGE_CHANNELS);
	msg->length   = LO8(DMX_STORAGE_CHANNELS);
	for(uint8_t i=0;i<DMX_STORAGE_CHANNELS;i++)
		(&(msg->dataStart))[i]=get_dmx_channel_slot(artnet_inputUniverse1,i,artnet_conn_id);
	/* broadcast the packet */
	artnet_send(sizeof(struct artnet_dmx)+DMX_STORAGE_CHANNELS);
}
int16_t parse_cmd_artnet_pollreply (int8_t*cmd, int8_t*output, uint16_t len)
{
	artnet_sendPollReply();
	return ECMD_FINAL_OK;
}
void processPollPacket(struct artnet_poll *poll) {
	if ((poll->talkToMe & 2) == 2) {
		artnet_sendPollReplyOnChange = TRUE;
	} else {
		artnet_sendPollReplyOnChange = FALSE;
	}

	if ((poll->talkToMe & 1) == 1) {
		artnet_pollReplyTarget = *uip_hostaddr;
	} else {
		artnet_pollReplyTarget = (uint64_t)0xffffffff;
	}

	artnet_sendPollReply();
}

void artnet_main(void) {	
	if(get_dmx_universe_state(artnet_inputUniverse1,artnet_conn_id) == DMX_NEWVALUES && artnet_connected == TRUE)
	{	
		ARTNET_DEBUG("Universe has changed, sending artnet data!\r\n");
		artnet_sendDmxPacket();
	}
}


/* ----------------------------------------------------------------------------
 * receive Art-Net packet
 */
void artnet_get(void) {
	struct artnet_header *header;

	header = (struct artnet_header *)uip_appdata;

	/* check the id */
	if ( (header->id[0] != 'A') ||
			(header->id[1] != 'r') ||
			(header->id[2] != 't') ||
			(header->id[3] != '-') ||
			(header->id[4] != 'N') ||
			(header->id[5] != 'e') ||
			(header->id[6] != 't') ||
			(header->id[7] !=  0 )    )
	{
		ARTNET_DEBUG("Wrong ArtNet header, discarded\r\n");
		artnet_status = RC_PARSE_FAIL;
		return;
	}

	if (header->opcode == OP_POLL) {
		struct artnet_poll *poll;

		ARTNET_DEBUG("Received artnet poll packet!\r\n");
		poll = (struct artnet_poll *)uip_appdata;
		processPollPacket(poll);

	} else if (header->opcode == OP_POLLREPLY) {
		ARTNET_DEBUG("Received artnet poll reply packet!\r\n");
	} else if (header->opcode == OP_OUTPUT) {
		struct artnet_dmx *dmx;

		ARTNET_DEBUG("Received artnet output packet!\r\n");
		dmx = (struct artnet_dmx *)uip_appdata;

		if (dmx->universe == ((artnet_subNet << 4) | artnet_outputUniverse1)) {
			if (artnet_dmxDirection == 0) {
				uint16_t len = (dmx->lengthHi << 8) + dmx->length;
				ARTNET_DEBUG ("Updating %d channels ...\n", len);
#ifdef DMX_STORAGE_SUPPORT
				set_dmx_channels(&dmx->dataStart,CONF_ARTNET_OUTUNIVERSE,len);
#endif
				if (artnet_sendPollReplyOnChange == TRUE) {
					artnet_pollReplyCounter++;
					artnet_sendPollReply();
				}
			}
		}
	} else if (header->opcode == OP_ADDRESS) {
		struct artnet_address *address;

		ARTNET_DEBUG("Received artnet address packet!\r\n");
		address = (struct artnet_address *)uip_appdata;

	} else if (header->opcode == OP_IPPROG) {
		struct artnet_ipprog *ipprog;

		ARTNET_DEBUG("Received artnet ip prog packet!\r\n");
		ipprog = (struct artnet_ipprog *)uip_appdata;
	}
}
#endif /* ARTNET_SUPPORT */

/*
   -- Ethersex META --
   header(protocols/artnet/artnet.h)
   net_init(artnet_init)
   mainloop(artnet_main)
   block(Miscelleanous)
   ecmd_feature(artnet_pollreply, "artnet test",,artnet test)
 */
