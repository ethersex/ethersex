/*
 * Remote Infrared-Multiprotokoll-Decoder
 *
 * for additional information please
 * see https://www.mikrocontroller.net/articles/Remote_IRMP
 *
 * Copyright (c) 2020 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "core/debug.h"
#include "core/queue/queue.h"
#include "protocols/mqtt/mqtt.h"
#include "irmp.h"
#include "irmp_mqtt.h"


#ifdef DEBUG_IRMP
#define IRMPDEBUG(s, ...)   debug_printf("irmp: " s "\n", ## __VA_ARGS__)
#else
#define IRMPDEBUG(...)      do { } while(0)
#endif

#define IRMP_MQTT_QUEUE_LEN 5

/* Prefix */
#define IRMP_MQTT_TOPIC           MQTT_CONF_PREFIX "/" MQTT_CONF_CLIENTID "/ir"
#define IRMP_MQTT_TX_TOPIC	  IRMP_MQTT_TOPIC "/tx"
#define IRMP_MQTT_SUBSCRIBE       IRMP_MQTT_TX_TOPIC "/#"
#define IRMP_MQTT_RX_TOPIC	  IRMP_MQTT_TOPIC "/rx"
#define IRMP_MQTT_PUBLISH_FORMAT  "%"PRIu16",0x%04"PRIx16",0x%04"PRIx16",0x%02"PRIx8

#ifdef IRMP_RX_SUPPORT
static Queue irmp_mqtt_rx_queue = {.limit = IRMP_MQTT_QUEUE_LEN };

uint8_t
irmp_mqtt_enqueue_rx(irmp_data_t * data)
{
  return queue_push(data, &irmp_mqtt_rx_queue);
}

static void
irmp_mqtt_poll_cb(void)
{
  irmp_data_t *irmp_data_p = queue_pop(&irmp_mqtt_rx_queue);
  if (irmp_data_p == NULL)
    return;

  IRMPDEBUG("rx proto %02" PRId8 " (%S), address 0x%04" PRIX16
            ", command 0x%04" PRIX16 ", flags 0x%02" PRIX8 "\n",
            irmp_data_p->protocol,
            pgm_read_word(&irmp_proto_names[irmp_data_p->protocol]),
            irmp_data_p->address, irmp_data_p->command, irmp_data_p->flags);

  size_t len = (size_t) snprintf_P(NULL, 0, PSTR(IRMP_MQTT_PUBLISH_FORMAT),
                                   irmp_data_p->protocol,
                                   irmp_data_p->address, irmp_data_p->command,
                                   irmp_data_p->flags);

  char *buf = malloc(len);
  if (buf == NULL)
    goto out;

  snprintf_P(buf, len, PSTR(IRMP_MQTT_PUBLISH_FORMAT),
             irmp_data_p->protocol, irmp_data_p->address,
             irmp_data_p->command, irmp_data_p->flags);

  mqtt_construct_publish_packet_P(PSTR(IRMP_MQTT_RX_TOPIC), buf, len, false);
  free(buf);
out:
  free(irmp_data_p);
}
#endif

#ifdef IRMP_TX_SUPPORT
static void
irmp_mqtt_publish_cb(const char *topic, uint16_t topic_length,
                     const void *payload, uint16_t payload_length,
                     bool retained)
{
  if (payload_length == 0)
    return;

  irmp_data_t data;
  int res =
    sscanf_P(payload, PSTR("%" PRIu8 ",%" SCNi16 ",%" SCNi16 ",%" SCNi16),
             &data.protocol, &data.address, &data.command,
             &data.flags);
  if (res < 3)
    return;
  if (res == 3)
    data.flags = 0;

  irmp_write(&data);
}
#endif

static void
irmp_mqtt_connack_cb(void)
{
  mqtt_construct_subscribe_packet_P(PSTR(IRMP_MQTT_SUBSCRIBE));
}

static const char irmp_mqtt_tx_topic[] PROGMEM = IRMP_MQTT_TX_TOPIC;

const mqtt_callback_config_t irmp_mqtt_module_config PROGMEM = {
  .topic = (const char *const *) irmp_mqtt_tx_topic,
#ifdef IRMP_TX_SUPPORT
  .connack_callback = irmp_mqtt_connack_cb,
#endif
#ifdef IRMP_RX_SUPPORT
  .poll_callback = irmp_mqtt_poll_cb,
#endif
#ifdef IRMP_TX_SUPPORT
  .publish_callback = irmp_mqtt_publish_cb
#endif
};

/*
  -- Ethersex META --
  header(hardware/ir/irmp/irmp_mqtt.h)
  mqtt_conf(irmp_mqtt_module_config)
*/
