/*
 * Copyright (c) 2014 by Philip Matura <ike@tura-home.de>
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
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

#ifndef HAVE_MQTT_H
#define HAVE_MQTT_H


#include "config.h"
#include "protocols/uip/uip.h"
#include <stdint.h>
#include <stdbool.h>


// KEEP ALIVE

#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 60
#endif

// SENDBUFFER LENGTH

// don't increase this one over 256 without increasing the state variables in
// mqtt.c to 16 bit
#ifndef MQTT_SENDBUFFER_LENGTH
#define MQTT_SENDBUFFER_LENGTH 256
#endif


// CONSTANTS

#define TIMER_TICKS_PER_SECOND 50

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)


// a macro for defining uip_ipaddr_t structures at compile time
#include <protocols/uip/uip.h>
#define const_uip_ipaddr(addr0,addr1,addr2,addr3) \
                     HTONS(((addr0) << 8) | (addr1)), HTONS(((addr2) << 8) | (addr3))


// CONFIG STRUCTURE
typedef void (*connack_callback) (void);
typedef void (*poll_callback) (void);
typedef void (*close_callback) (void);
typedef void (*publish_callback) (char const *topic, uint16_t topic_length,
                                  void const *payload,
                                  uint16_t payload_length);
typedef struct
{
  // see mqtt.c for explanation
  connack_callback connack_callback;
  poll_callback poll_callback;
  close_callback close_callback;
  publish_callback publish_callback;
} mqtt_callback_config_t;

typedef struct
{
  char const *client_id;
  char const *user;
  char const *pass;
  char const *will_topic;       // A value != NULL enables the will feature
  uint8_t will_qos;
  bool will_retain;
  char const *will_message;
  uip_ipaddr_t target_ip;

  // Pointer to an array of (char const*) of topic strings to be automatically
  // subscribed to after a connection is established. The array is assumed to
  // be NULL-terminated.
  char const *const *auto_subscribe_topics;
} mqtt_connection_config_t;


// PUBLIC FUNCTIONS

void mqtt_set_connection_config(mqtt_connection_config_t const *const config);
uint8_t mqtt_register_callback(mqtt_callback_config_t const *const callbacks);
void mqtt_unregister_callback(uint8_t slot_id);
bool mqtt_is_connected(void);

// put a packet in the mqtt send queue
// return false if there is not enough buffer space
bool mqtt_construct_publish_packet(char const *topic, const void *payload,
                                   uint16_t payload_length, bool retain);
bool mqtt_construct_subscribe_packet(char const *topic);
bool mqtt_construct_unsubscribe_packet(char const *topic);
bool mqtt_construct_zerolength_packet(uint8_t msg_type);
bool mqtt_construct_ack_packet(uint8_t msg_type, uint16_t msgid);


// INTERNAL
void mqtt_periodic(void);

#ifdef MQTT_STATIC_CONF
void mqtt_set_static_conf(void);
#else
#define mqtt_set_static_conf(...)
#endif


#endif /* HAVE_MQTT_H */
