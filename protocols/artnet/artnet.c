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
#include <avr/interrupt.h>

#include "config.h"
#include "core/bool.h"
#include "core/bit-macros.h"
#include "protocols/artnet/artnet.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "services/dmx-storage/dmx_storage.h"
#ifdef ARTNET_SUPPORT


/* ----------------------------------------------------------------------------
 *global variables
 */

uint8_t artnet_subNet = SUBNET_DEFAULT;
uint8_t artnet_outputUniverse;
uint8_t artnet_inputUniverse;
uint8_t artnet_sendPollReplyOnChange = TRUE;
uint64_t artnet_pollReplyTarget = (uint64_t) 0xffffffff;
uint32_t artnet_pollReplyCounter = 0;
uint8_t artnet_status = RC_POWER_OK;
char artnet_shortName[18] = { '\0' };
char artnet_longName[64] = { '\0' };

uint16_t artnet_port = CONF_ARTNET_PORT;
uint8_t artnet_netConfig = NETCONFIG_DEFAULT;

volatile uint16_t artnet_dmxChannels = 0;
volatile uint8_t artnet_dmxTransmitting = FALSE;
volatile uint8_t artnet_dmxInChanged = FALSE;
volatile uint8_t artnet_dmxInComplete = FALSE;
int8_t artnet_conn_id = 0;
uint8_t artnet_connected = 0;
uint8_t artnet_dmxDirection = 0;

const char artnet_ID[8] PROGMEM = "Art-Net";

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
void
artnet_init(void)
{

  ARTNET_DEBUG("Init\n");
  /* read Art-Net port */
  artnet_port = CONF_ARTNET_PORT;
  /* read netconfig */
  artnet_netConfig = NETCONFIG_DEFAULT;

  /* read subnet */
  artnet_subNet = SUBNET_DEFAULT;
  artnet_inputUniverse = CONF_ARTNET_INUNIVERSE;
  artnet_outputUniverse = CONF_ARTNET_OUTUNIVERSE;
  strcpy_P(artnet_shortName, PSTR("e6ArtNode"));
  strcpy_P(artnet_longName, PSTR("e6ArtNode hostname: " CONF_HOSTNAME));

  /* dmx storage connection */
  artnet_conn_id = dmx_storage_connect(artnet_inputUniverse);
  if (artnet_conn_id != -1)
  {
    artnet_connected = TRUE;
    ARTNET_DEBUG("Connection to dmx-storage established! id:%d\r\n",
                 artnet_conn_id);
  }
  else
  {
    artnet_connected = FALSE;
    ARTNET_DEBUG("Connection to dmx-storage couldn't be established!\r\n");
  }

  /* net_init */
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
artnet_send(uint16_t len)
{
  uip_udp_conn_t artnet_conn;
  artnet_conn.ripaddr[0] = uip_hostaddr[0] | ~uip_netmask[0];
  artnet_conn.ripaddr[1] = uip_hostaddr[1] | ~uip_netmask[1];
  artnet_conn.rport = HTONS(artnet_port);
  artnet_conn.lport = HTONS(artnet_port);
  uip_udp_conn = &artnet_conn;

  uip_slen = len;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}


/* ----------------------------------------------------------------------------
 * send an ArtPollReply packet
 */
void
artnet_sendPollReply(void)
{

  /* prepare artnet PollReply packet */
  struct artnet_pollreply *msg =
    (struct artnet_pollreply *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
  memset(msg, 0, sizeof(struct artnet_pollreply));
  ARTNET_DEBUG("PollReply allocated\n");
  strncpy_P((char *) msg->id, artnet_ID, 8);

  msg->opcode = OP_POLLREPLY;

  msg->versionInfoH = (FIRMWARE_VERSION >> 8) & 0xFF;
  msg->versionInfo = FIRMWARE_VERSION & 0xFF;

  msg->subSwitchH = 0;
  msg->subSwitch = artnet_subNet & 15;

  /* Report as 'AvrArtNode' http://www.dmxcontrol.de/wiki/Art-Net-Node_f%C3%BCr_25_Euro */
  msg->oem = 0x08b1;
  msg->ubeaVersion = 0;
  msg->status = 0;
  /* Report as Manufacturer "ESTA" http://tsp.plasa.org/tsp/working_groups/CP/mfctrIDs.php */
  msg->estaMan = 0xFFFF;
  strcpy(msg->shortName, artnet_shortName);
  strcpy(msg->longName, artnet_longName);
  sprintf(msg->nodeReport, "#%04X [%04u] e6ArtNode is ready", artnet_status,
          (unsigned int) artnet_pollReplyCounter);

  msg->numPortsH = 0;
  msg->numPorts = 1;

  if (artnet_dmxDirection == 1)
    msg->portTypes[0] = PORT_TYPE_DMX_INPUT;
  else
    msg->portTypes[0] = PORT_TYPE_DMX_OUTPUT;

  if (artnet_dmxDirection != 1)
    msg->goodInput[0] = (1 << 3);
  else if (artnet_dmxChannels > 0)
    msg->goodInput[0] |= (1 << 7);

  msg->goodOutput[0] = (1 << 1);
  if (artnet_dmxTransmitting == TRUE)
    msg->goodOutput[0] |= (1 << 7);

  msg->swin[0] = (artnet_subNet & 15) * 16 | (artnet_inputUniverse & 15);
  msg->swout[0] = (artnet_subNet & 15) * 16 | (artnet_outputUniverse & 15);
  msg->style = STYLE_NODE;

  memcpy(msg->mac, uip_ethaddr.addr, 6);

  /* broadcast the packet */
  artnet_send(sizeof(struct artnet_pollreply));
}

/* ----------------------------------------------------------------------------
 * send an ArtDmx packet
 */
void
artnet_sendDmxPacket(void)
{
  static unsigned char sequence = 1;
  /* prepare artnet Dmx packet */
  struct artnet_dmx *msg =
    (struct artnet_dmx *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
  memset(msg, 0, sizeof(struct artnet_dmx));

  strncpy_P((char *) msg->id, artnet_ID, 8);

  msg->opcode = OP_OUTPUT;

  msg->versionH = 0;
  msg->version = PROTOCOL_VERSION;

  msg->sequence = sequence++;
  if (sequence == 0)
    sequence = 1;

  msg->physical = 1;
  msg->universe = ((artnet_subNet << 4) | artnet_inputUniverse);
  msg->lengthHi = HI8(DMX_STORAGE_CHANNELS);
  msg->length = LO8(DMX_STORAGE_CHANNELS);
  for (uint8_t i = 0; i < DMX_STORAGE_CHANNELS; i++)
    (&(msg->dataStart))[i] =
      get_dmx_channel_slot(artnet_inputUniverse, i, artnet_conn_id);
  /* broadcast the packet */
  artnet_send(sizeof(struct artnet_dmx) + DMX_STORAGE_CHANNELS);
}

int16_t
parse_cmd_artnet_pollreply(int8_t * cmd, int8_t * output, uint16_t len)
{
  artnet_sendPollReply();
  return ECMD_FINAL_OK;
}

void
processPollPacket(struct artnet_poll *poll)
{
  if ((poll->talkToMe & 2) == 2)
    artnet_sendPollReplyOnChange = TRUE;
  else
    artnet_sendPollReplyOnChange = FALSE;
  if ((poll->talkToMe & 1) == 1)
    artnet_pollReplyTarget = *uip_hostaddr;
  else
    artnet_pollReplyTarget = (uint64_t) 0xffffffff;
  artnet_sendPollReply();
}

void
artnet_main(void)
{
  if (get_dmx_universe_state(artnet_inputUniverse, artnet_conn_id) ==
      DMX_NEWVALUES && artnet_connected == TRUE)
  {
    ARTNET_DEBUG("Universe has changed, sending artnet data!\r\n");
    artnet_sendDmxPacket();
  }
}


/* ----------------------------------------------------------------------------
 * receive Art-Net packet
 */
void
artnet_get(void)
{
  struct artnet_header *header;

  header = (struct artnet_header *) uip_appdata;

  /* check the id */
  if (strncmp_P((char *) header->id, artnet_ID, 8))
  {
    ARTNET_DEBUG("Wrong ArtNet header, discarded\r\n");
    artnet_status = RC_PARSE_FAIL;
    return;
  }
  switch (header->opcode)
  {
    case OP_POLL:;
      struct artnet_poll *poll;

      ARTNET_DEBUG("Received artnet poll packet!\r\n");
      poll = (struct artnet_poll *) uip_appdata;
      processPollPacket(poll);
      break;
    case OP_POLLREPLY:;
      ARTNET_DEBUG("Received artnet poll reply packet!\r\n");
      break;
    case OP_OUTPUT:;
      struct artnet_dmx *dmx;

      ARTNET_DEBUG("Received artnet output packet!\r\n");
      dmx = (struct artnet_dmx *) uip_appdata;

      if (dmx->universe == ((artnet_subNet << 4) | artnet_outputUniverse))
      {
        if (artnet_dmxDirection == 0)
        {
          uint16_t len = ((dmx->lengthHi << 8) + dmx->length);
          set_dmx_channels(&dmx->dataStart, artnet_outputUniverse, len);
          if (artnet_sendPollReplyOnChange == TRUE)
          {
            artnet_pollReplyCounter++;
            artnet_sendPollReply();
          }
        }
      }
      break;
    case OP_ADDRESS:;
    case OP_IPPROG:;
      break;
    default:
      ARTNET_DEBUG("Received an invalid artnet packet!\r\n");
      break;
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
