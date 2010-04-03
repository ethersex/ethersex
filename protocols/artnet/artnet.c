/*
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 *
 * Author:         Stefan Krupop <mail@stefankrupop.de>
 *                 Dirk Pannenbecker <dp@sd-gp.de>
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
#include "protocols/artnet/artnet.h"
#include "protocols/dmx/dmx.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#ifdef STELLA_SUPPORT
#include "services/stella/stella.h"
#endif

#ifdef ARTNET_SUPPORT


#include <stdio.h>
#include <avr/interrupt.h>

#ifndef TRUE
#define TRUE			1
#endif
#ifndef FALSE
#define FALSE			0
#endif

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
#define INUNIVERSE_DEFAULT	1
#define OUTUNIVERSE_DEFAULT	0
//#define PORT_DEFAULT		0x1936
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
 unsigned char  ip[4];
 unsigned short port;
};

struct artnet_header {
 unsigned char  id[8];
 unsigned short opcode;
};

struct artnet_poll {
 unsigned char  id[8];
 unsigned short opcode;
 unsigned char  versionH;
 unsigned char  version;
 unsigned char  talkToMe;
 unsigned char  pad;
};

struct artnet_pollreply {
 unsigned char  id[8];
 unsigned short opcode;
 struct artnet_packet_addr addr;
 unsigned char  versionInfoH;
 unsigned char  versionInfo;
 unsigned char  subSwitchH;
 unsigned char  subSwitch;
 unsigned short oem;
 /* unsigned char oemH; */
 /* unsigned char oem; */
 unsigned char  ubeaVersion;
 unsigned char  status;
 unsigned short estaMan;
 char           shortName[SHORT_NAME_LENGTH];
 char           longName[LONG_NAME_LENGTH];
 char           nodeReport[LONG_NAME_LENGTH];
 unsigned char  numPortsH;
 unsigned char  numPorts;
 unsigned char  portTypes[MAX_NUM_PORTS];
 unsigned char  goodInput[MAX_NUM_PORTS];
 unsigned char  goodOutput[MAX_NUM_PORTS];
 unsigned char  swin[MAX_NUM_PORTS];
 unsigned char  swout[MAX_NUM_PORTS];
 unsigned char  swVideo;
 unsigned char  swMacro;
 unsigned char  swRemote;
 unsigned char  spare1;
 unsigned char  spare2;
 unsigned char  spare3;
 unsigned char  style;
 unsigned char  mac[6];
 unsigned char  filler[32];
};

struct artnet_ipprog {
 unsigned char  id[8];
 unsigned short opcode;
 unsigned char  versionH;
 unsigned char  version;
 unsigned char  filler1;
 unsigned char  filler2;
 unsigned char  command;
 unsigned char  filler3;
 unsigned char  progIp[4];
 unsigned char  progSm[4];
 unsigned char  progPort[2];
 unsigned char  spare[8];
};

struct artnet_ipprogreply {
 unsigned char  id[8];
 unsigned short opcode;
 unsigned char  versionH;
 unsigned char  version;
 unsigned char  filler1;
 unsigned char  filler2;
 unsigned char  filler3;
 unsigned char  filler4;
 unsigned char  progIp[4];
 unsigned char  progSm[4];
 unsigned char  progPort[2];
 unsigned char  spare[8];
};

struct artnet_address {
 unsigned char  id[8];
 unsigned short opcode;
 unsigned char  versionH;
 unsigned char  version;
 unsigned char  filler1;
 unsigned char  filler2;
 char           shortName[SHORT_NAME_LENGTH];
 char           longName[LONG_NAME_LENGTH];
 unsigned char  swin[MAX_NUM_PORTS];
 unsigned char  swout[MAX_NUM_PORTS];
 unsigned char  subSwitch;
 unsigned char  swVideo;
 unsigned char  command;
};

struct artnet_dmx {
 unsigned char  id[8];
 unsigned short opcode;
 unsigned char  versionH;
 unsigned char  version;
 unsigned char  sequence;
 unsigned char  physical;
 unsigned short universe;
 unsigned char  lengthHi;
 unsigned char  length;
 unsigned char  dataStart;
};

/* ----------------------------------------------------------------------------
 *global variables
 */
enum {BREAK, STARTB, DATA, STOPPED};

unsigned char  artnet_subNet = SUBNET_DEFAULT;
unsigned char  artnet_outputUniverse1 = OUTUNIVERSE_DEFAULT;
unsigned char  artnet_inputUniverse1 = INUNIVERSE_DEFAULT;
unsigned char  artnet_sendPollReplyOnChange = FALSE;
unsigned long  artnet_pollReplyTarget = (unsigned long)0xffffffff;
unsigned int   artnet_pollReplyCounter = 0;
unsigned char  artnet_status = RC_POWER_OK;
char           artnet_shortName[18];
char           artnet_longName[64];
unsigned short artnet_port = CONF_ARTNET_PORT;
unsigned char  artnet_netConfig = NETCONFIG_DEFAULT;

volatile unsigned char  artnet_dmxUniverse[MAX_CHANNELS];
volatile unsigned short artnet_dmxChannels = 0;
volatile unsigned char  artnet_dmxTransmitting = FALSE;
volatile unsigned char  artnet_dmxInChanged = FALSE;
volatile unsigned char  artnet_dmxInComplete = FALSE;
unsigned char  artnet_dmxDirection = 0;
unsigned char  artnet_dmxRefreshTimer = REFRESH_INTERVAL;

/* ----------------------------------------------------------------------------
 * initialization of network settings
 */
void
artnet_netInit(void)
{
//  if (artnet_netConfig == 1) {
//   if (*((unsigned long*)&myip[0]) == IP(127,127,127,127)) {
//    #if USE_DHCP
//     ARTNET_DEBUG("Setting network address: Custom (DHCP)\r\n");
//     dhcp_init();
//     if (dhcp() != 0) {
//      ARTNET_DEBUG("DHCP fail\r\n");
//      /* use programmed value */
//      (*((unsigned long*)&myip[0])) = MYIP;
//  	 (*((unsigned long*)&netmask[0])) = NETMASK;
//     }
//    #else
//     ARTNET_DEBUG("Setting network address: Custom\r\n");
//     (*((unsigned long*)&myip[0])) = MYIP;
//     (*((unsigned long*)&netmask[0])) = NETMASK;
//    #endif //USE_DHCP
//   } else {
//    read_ip_addresses();
//   }
//  } else {
//   if (!(PINB & (1 << 0))) {
//    ARTNET_DEBUG("Setting network address: Art-Net 2.x.x.x standard\r\n");
//    myip[0] = 10;
//   } else {
//    ARTNET_DEBUG("Setting network address: Art-Net 10.x.x.x standard\r\n");
//    myip[0] = 2;
//   }
//   myip[1] = (((OEM_ID >> 8) & 0xFF) + (OEM_ID & 0xFF) + MYMAC4) & 0xFF;
//   myip[2] = MYMAC5;
//   myip[3] = MYMAC6;
//   (*((unsigned long*)&netmask[0])) = IP(255,0,0,0);
//  }
// 
//  /* calculate broadcast adress */
//  (*((unsigned long*)&broadcast_ip[0])) = (((*((unsigned long*)&myip[0])) & (*((unsigned long*)&netmask[0]))) | (~(*((unsigned long*)&netmask[0]))));
// 
//  /* remove any existing app from port */
//  kill_udp_app(artnet_port);
//  /* add port to stack with callback */
//  add_udp_app(artnet_port, (void(*)(unsigned char))artnet_get);

  artnet_net_init();

}

/* ----------------------------------------------------------------------------
 * initialization of Art-Net
 */
void artnet_init(void) {

    ARTNET_DEBUG("Init\n");
 /* read Art-Net port */
//  eeprom_read_block(&artnet_port, (unsigned char *)ARTNET_PORT_EEPROM_STORE, 2);
//  if (artnet_port == 0xFFFF) {
  artnet_port = CONF_ARTNET_PORT;
//  }

 /* read netconfig */
//  artnet_netConfig = eeprom_read_byte((unsigned char *)ARTNET_NETCONFIG_EEPROM_STORE);
//  if (artnet_netConfig == 0xFF) {
  artnet_netConfig = NETCONFIG_DEFAULT;
//  }

 /* read subnet */
//  artnet_subNet = eeprom_read_byte((unsigned char *)ARTNET_SUBNET_EEPROM_STORE);
//  if (artnet_subNet == 0xFF) {
  artnet_subNet = SUBNET_DEFAULT;
//  }

 /* read nr. of input universe */
//  artnet_inputUniverse1 = eeprom_read_byte((unsigned char *)ARTNET_INUNIVERSE_EEPROM_STORE);
//  if (artnet_inputUniverse1 == 0xFF) {
  artnet_inputUniverse1 = INUNIVERSE_DEFAULT;
//  }

 /* read nr. of output universe */
//  artnet_outputUniverse1 = eeprom_read_byte((unsigned char *)ARTNET_OUTUNIVERSE_EEPROM_STORE);
//  if (artnet_outputUniverse1 == 0xFF) {
  artnet_outputUniverse1 = OUTUNIVERSE_DEFAULT;
//  }

 /* read short name */
//  eeprom_read_block(&artnet_shortName, (unsigned char *)ARTNET_SHORTNAME_EEPROM_STORE, SHORT_NAME_LENGTH);
//  if ((*((unsigned long*)&artnet_shortName[0])) == 0xFFFFFFFF) {
  /* fill with zeroes */
  for (unsigned char i = 0; i < SHORT_NAME_LENGTH; i++) {
   artnet_shortName[i] = 0;
  }
  strcpy_P(artnet_shortName, PSTR("AvrArtNode"));
//  }
 artnet_shortName[SHORT_NAME_LENGTH - 1] = 0;

 /* read long name */
//  eeprom_read_block(&artnet_longName, (unsigned char *)ARTNET_LONGNAME_EEPROM_STORE, LONG_NAME_LENGTH);
//  if ((*((unsigned long*)&artnet_longName[0])) == 0xFFFFFFFF) {
  /* fill with zeroes */
  for (unsigned char i = 0; i < LONG_NAME_LENGTH; i++) {
   artnet_longName[i] = 0;
  }
  strcpy_P(artnet_longName, PSTR("AVR based Art-Net node"));
//  }
 artnet_longName[LONG_NAME_LENGTH - 1] = 0;

//  ARTNET_DEBUG("net init\n");
 artnet_netInit();

 /* annouce that we are here  */
 ARTNET_DEBUG("send PollReply\n");
 artnet_sendPollReply();

 /* enable PollReply on changes */
 artnet_sendPollReplyOnChange = TRUE;

 ARTNET_DEBUG("init complete\n");
 return;
}

static void
artnet_send (uint16_t len)
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

 memcpy (msg->addr.ip, uip_hostaddr, 4);
 msg->addr.port = artnet_port;
 msg->versionInfoH = (FIRMWARE_VERSION >> 8) & 0xFF;
 msg->versionInfo = FIRMWARE_VERSION & 0xFF;

 msg->subSwitchH = 0;
 msg->subSwitch = artnet_subNet & 15;

 msg->oem = (unsigned short)OEM_ID;
 msg->ubeaVersion = 0;
 msg->status = 0;
 msg->estaMan = 'D' * 256 + 'P';
 strcpy(msg->shortName, artnet_shortName);
 strcpy(msg->longName, artnet_longName);
 sprintf(msg->nodeReport, "#%04X [%04u] AvrArtNode is ready", artnet_status, artnet_pollReplyCounter);

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

int16_t 
parse_cmd_artnet_pollreply (char *cmd, char *output, uint16_t len)
{
  artnet_sendPollReply();
  return ECMD_FINAL_OK;
}

// /* ----------------------------------------------------------------------------
//  * send an ArtIpProgReply packet
//  */
// void artnet_sendIpProgReply(unsigned long target) {
//     /* prepare artnet IpProgReply packet */
//     struct artnet_ipprogreply *msg = uip_appdata;
//     memset(msg, 0, sizeof(struct artnet_ipprogreply));
// 
//  msg->id[0] = 'A';
//  msg->id[1] = 'r';
//  msg->id[2] = 't';
//  msg->id[3] = '-';
//  msg->id[4] = 'N';
//  msg->id[5] = 'e';
//  msg->id[6] = 't';
//  msg->id[7] =  0 ;
//  msg->opcode = OP_IPPROGREPLY;
// 
//  msg->versionH = 0;
//  msg->version = PROTOCOL_VERSION;
// 
//  uip_ipaddr_t myip, netmask;
//  uip_gethostaddr(&myip);
//  uip_getnetmask(&netmask);
//  for (unsigned char i = 0; i < 4; i++) {
//   msg->progIp[i] = myip[i];
//   msg->progSm[i] = netmask[i];
//  }
//  msg->progPort[0] = (artnet_port >> 8) & 0xff;
//  msg->progPort[1] = artnet_port & 0xff;
// 
//     /* broadcast the packet */
//     uip_udp_send(sizeof(struct msg));
// }
// 
// /* ----------------------------------------------------------------------------
//  * send an ArtDmx packet
//  */
// void artnet_sendDmxPacket(void) {
//  static unsigned char sequence = 1;
//     /* prepare artnet Dmx packet */
//     struct artnet_dmx *msg = uip_appdata;
//     memset(msg, 0, sizeof(struct artnet_dmx));
// 
//  msg->id[0] = 'A';
//  msg->id[1] = 'r';
//  msg->id[2] = 't';
//  msg->id[3] = '-';
//  msg->id[4] = 'N';
//  msg->id[5] = 'e';
//  msg->id[6] = 't';
//  msg->id[7] =  0 ;
//  msg->opcode = OP_OUTPUT;
// 
//  msg->versionH = 0;
//  msg->version = PROTOCOL_VERSION;
// 
//  msg->sequence = sequence++;
//  if (sequence == 0) {
//   sequence = 1;
//  }
// 
//  msg->physical = 1;
//  msg->universe = ((artnet_subNet << 4) | artnet_inputUniverse1);
// 
//  msg->lengthHi = (artnet_dmxChannels >> 8) & 0xFF;
//  msg->length = artnet_dmxChannels & 0xFF;
// 
//  memcpy(&(msg->dataStart), (unsigned char *)&artnet_dmxUniverse[0], artnet_dmxChannels);
// 
//     /* broadcast the packet */
//     uip_udp_send(sizeof(struct msg));
// }
// 
// /* ----------------------------------------------------------------------------
//  * process an ArtPoll packet
//  */
// void processPollPacket(struct artnet_poll *poll) {
//  if ((poll->talkToMe & 2) == 2) {
//   artnet_sendPollReplyOnChange = TRUE;
//  } else {
//   artnet_sendPollReplyOnChange = FALSE;
//  }
// 
//  if ((poll->talkToMe & 1) == 1) {
//   struct IP_Header *ip;
//   ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
//   artnet_pollReplyTarget = ip->IP_Srcaddr;
//  } else {
//   artnet_pollReplyTarget = (unsigned long)0xffffffff;
//  }
// 
//  artnet_sendPollReply();
// }
// 
// /* ----------------------------------------------------------------------------
//  * process an ArtAddress packet
//  */
// void processAddressPacket(struct artnet_address *address) {
//  unsigned char changed = 0;
// 
//  if (address->shortName[0] != 0) {
//   /* set short name */
//   strcpy(artnet_shortName, address->shortName);
//   eeprom_write_block(&artnet_shortName, (unsigned char *)ARTNET_SHORTNAME_EEPROM_STORE, SHORT_NAME_LENGTH);
//   artnet_status = RC_SH_NAME_OK;
//   changed = 1;
//  }
// 
//  if (address->longName[0] != 0) {
//   /* set long name */
//   strcpy(artnet_longName, address->longName);
//   eeprom_write_block(&artnet_longName, (unsigned char *)ARTNET_LONGNAME_EEPROM_STORE, LONG_NAME_LENGTH);
//   artnet_status = RC_LO_NAME_OK;
//   changed = 1;
//  }
// 
//  if (address->swin[0] == 0) {
//   /* reset input universe nr. */
//   artnet_inputUniverse1 = INUNIVERSE_DEFAULT;
//   eeprom_write_byte((unsigned char *)ARTNET_INUNIVERSE_EEPROM_STORE, artnet_inputUniverse1);
//   changed = 1;
//  } else if ((address->swin[0] & 128) == 128) {
//   /* set input universe nr. */
//   artnet_inputUniverse1 = address->swin[0] & 0xF;
//   eeprom_write_byte((unsigned char *)ARTNET_INUNIVERSE_EEPROM_STORE, artnet_inputUniverse1);
//   changed = 1;
//  }
// 
//  if (address->swout[0] == 0) {
//   /* reset output universe nr. */
//   artnet_outputUniverse1 = OUTUNIVERSE_DEFAULT;
//   eeprom_write_byte((unsigned char *)ARTNET_OUTUNIVERSE_EEPROM_STORE, artnet_outputUniverse1);
//   changed = 1;
//  } else if ((address->swout[0] & 128) == 128) {
//   /* set output universe nr. */
//   artnet_outputUniverse1 = address->swout[0] & 0xF;
//   eeprom_write_byte((unsigned char *)ARTNET_OUTUNIVERSE_EEPROM_STORE, artnet_outputUniverse1);
//   changed = 1;
//  }
// 
//  if (address->subSwitch == 0) {
//   /* reset subnet */
//   artnet_subNet = SUBNET_DEFAULT;
//   eeprom_write_byte((unsigned char *)ARTNET_SUBNET_EEPROM_STORE, artnet_subNet);
//   changed = 1;
//  } else if ((address->subSwitch & 128) == 128) {
//   /* set subnet */
//   artnet_subNet = address->subSwitch & 0xF;
//   eeprom_write_byte((unsigned char *)ARTNET_SUBNET_EEPROM_STORE, artnet_subNet);
//   changed = 1;
//  }
// 
//  /* send PollReply when something changed */
//  if (changed == 1 && artnet_sendPollReplyOnChange == TRUE) {
//   artnet_pollReplyCounter++;
//   artnet_sendPollReply();
//  }
// }
// 
// /* ----------------------------------------------------------------------------
//  * process an ArtIpProg packet
//  */
// void processIpProgPacket(struct artnet_ipprog *ipprog) {
// 
//  if ((ipprog->command & 128) == 128) {	/* enable programming */
//   /* program port */
//   if ((ipprog->command & 1) == 1) {
//    kill_udp_app(artnet_port);
//    artnet_port = (ipprog->progPort[0] << 8) | ipprog->progPort[1];
//    ARTNET_DEBUG("IPPROG: New port: %x (%x, %x)\r\n", artnet_port, ipprog->progPort[0], ipprog->progPort[1]);
//    eeprom_write_block(&artnet_port, (unsigned char *)ARTNET_PORT_EEPROM_STORE, 2);
//    add_udp_app(artnet_port, (void(*)(unsigned char))artnet_get);
//   }
// 
//   /* program subnet */
//   if ((ipprog->command & 2) == 2) {
//    (*((unsigned long*)&netmask[0])) = (*((unsigned long*)&ipprog->progSm[0]));
//    for (unsigned char count = 0; count<4; count++) {
//     eeprom_busy_wait ();
//     eeprom_write_byte((unsigned char *)(NETMASK_EEPROM_STORE + count),netmask[count]);
//    }
//    artnet_netConfig = 1;
//   }
// 
//   /* program ip */
//   if ((ipprog->command & 4) == 4) {
//    (*((unsigned long*)&myip[0])) = (*((unsigned long*)&ipprog->progIp[0]));
//    for (unsigned char count = 0; count<4; count++) {
//     eeprom_busy_wait ();
//     eeprom_write_byte((unsigned char *)(IP_EEPROM_STORE + count),myip[count]);
//    }
//    artnet_netConfig = 1;
//   }
// 
//   /* reset to default */
//   if ((ipprog->command & 8) == 8) {
//    kill_udp_app(artnet_port);
//    artnet_port = CONF_ARTNET_PORT;
//    eeprom_write_block(&artnet_port, (unsigned char *)ARTNET_PORT_EEPROM_STORE, 2);
//    add_udp_app(artnet_port, (void(*)(unsigned char))artnet_get);
//    artnet_netConfig = 0;
//   }
//   eeprom_write_byte((unsigned char *)ARTNET_NETCONFIG_EEPROM_STORE, artnet_netConfig);
//  }
// 
//  struct IP_Header *ip;
//  ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
// 
//  artnet_netInit();
// 
//  artnet_sendIpProgReply(ip->IP_Srcaddr);
// }

void artnet_main(void) {
 if (artnet_dmxInComplete == TRUE) {
  if (artnet_dmxInChanged == TRUE) {
//    artnet_sendDmxPacket();
   artnet_dmxInChanged = FALSE;
  }
  artnet_dmxInComplete = FALSE;
  artnet_dmxChannels = 0;
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

//   processPollPacket(poll);
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
		#ifdef DMX_SUPPORT
			if (len > CONF_DMX_MAX_CHAN) len = CONF_DMX_MAX_CHAN;
			memcpy (dmx_data, &dmx->dataStart, len);
			dmx_prg = 0;
		#endif  /* DMX_SUPPORT */
		#ifdef STELLA_SUPPORT
			stella_dmx(&dmx->dataStart, len);
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

//   processAddressPacket(address);
 } else if (header->opcode == OP_IPPROG) {
  struct artnet_ipprog *ipprog;

  ARTNET_DEBUG("Received artnet ip prog packet!\r\n");
  ipprog = (struct artnet_ipprog *)uip_appdata;

//   processIpProgPacket(ipprog);
 }
}

// /* ----------------------------------------------------------------------------
//  * Called by timer, check changes
//  */
// void artnet_tick(void) {
//  unsigned char changed = 0;
// 
//  /* set DMX direction */
//  if (!(PINB & (1 << 3))) {
//   if (artnet_dmxDirection != 1) {
//    artnet_dmxDirection = 1;
//    artnet_dmxTransmitting = FALSE;
// 
//    /* setup USART */
//    PORTD &= ~(1 << 3);
//    UBRR   = (F_CPU / (250000 * 16L) - 1);
//    UCSRC  = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
//    UCSRB  = (1<<RXEN) | (1<<RXCIE);
// 
//    changed = 1;
//   }
//  } else {
//   if (artnet_dmxDirection != 0) {
//    artnet_dmxDirection = 0;
//    changed = 1;
//   }
//  }
// 
//  /* send PollReply when something changed */
//  if (changed == 1 && artnet_sendPollReplyOnChange == TRUE) {
//   artnet_pollReplyCounter++;
//   artnet_sendPollReply();
//  }
// 
//  if (artnet_dmxDirection == 1) {
//   if (artnet_dmxRefreshTimer > 0) {
//    artnet_dmxRefreshTimer--;
//   } else {
//    if (artnet_dmxChannels > 0) {
//     ARTNET_DEBUG("Refreshing DMX packet\r\n");
//     artnet_sendDmxPacket();
//    }
//    artnet_dmxRefreshTimer = REFRESH_INTERVAL - 1;
//   }
//  }
// }
// 
// /* ----------------------------------------------------------------------------
//  * DMX transmission
//  */
// ISR (USART_TXC_vect) {
//  static unsigned char  dmxState = BREAK;
//  static unsigned short curDmxCh = 0;
// 
//  if (dmxState == STOPPED) {
//   if (artnet_dmxTransmitting == TRUE) {
//    dmxState = BREAK;
//   }
//  } else if (dmxState == BREAK) {
//   UBRR = (F_CPU / (50000 * 16L) - 1);
//   UDR      = 0;					/* send break */
//   dmxState = STARTB;
//  } else if (dmxState == STARTB) {
//   UBRR = (F_CPU / (250000 * 16L) - 1);
//   UDR      = 0;					/* send start byte */
//   dmxState = DATA;
//   curDmxCh = 0;
//  } else {
//   _delay_us(IBG);
//   UDR      = artnet_dmxUniverse[curDmxCh++];	/* send data */
//   if (curDmxCh == artnet_dmxChannels) {
//    if (artnet_dmxTransmitting == TRUE) {
//     dmxState = BREAK; 				/* new break if all ch sent */
//    } else {
//     dmxState = STOPPED;
//    } 
//   }
//  }
// }
// 
// /* ----------------------------------------------------------------------------
//  * DMX reception
//  */
// ISR (USART_RXC_vect) {
//  static unsigned char  dmxState = 0;
//  static unsigned short dmxFrame = 0;
//  unsigned char status = UCSRA; 	/* status register must be read prior to UDR (because of 2 byte fifo buffer) */
//  unsigned char byte = UDR; 	/* immediately catch data from i/o register to enable reception of the next byte */
// 
//  if ((byte == 0) && (status & (1<<FE))) {		/* BREAK detected (Framing Error) */
//   dmxState = 1;
//   dmxFrame = 0;
//   if (artnet_dmxChannels > 0) {
//    artnet_dmxInComplete = TRUE;
//   }
//  } else if (dmxFrame == 0) {				/* Start code test */
//   if ((byte == 0) && (dmxState == 1)) {			/* valid SC detected */
//    dmxState = 2;
//   }
//   dmxFrame = 1;
//  } else {
//   if ((dmxState == 2) && (dmxFrame <= MAX_CHANNELS)) {	/* addressed to us */
//    if (artnet_dmxUniverse[dmxFrame - 1] != byte) {
//     artnet_dmxUniverse[dmxFrame - 1] = byte;
// 	artnet_dmxInChanged = TRUE;
//    }
//    if (dmxFrame > artnet_dmxChannels) {
//     artnet_dmxChannels = dmxFrame;
//    }
//   }
//   dmxFrame++;
//  }
// }

#endif /* ARTNET_SUPPORT */

/*
  -- Ethersex META --
  header(protocols/artnet/artnet.h)
  net_init(artnet_init)
  block(Miscelleanous)
  ecmd_feature(artnet_pollreply, "artnet test",,artnet test)
*/
