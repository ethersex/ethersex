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


/*
 * A simple MQTT client.
 *
 * With inspiration from
 * https://github.com/knolleary/pubsubclient
 *
 * *Usage*:
 *
 *  - create a mqtt_connection_config_t structure somewhere
 *    and call mqtt_set_connection_config(.) or use the static
 *    connection setting in menuconfig
 *  - the connection will be established in the next uip_poll cycle
 *  - create a mqtt_callback_config_t structure in PROGMEM and supply your
 *    callback functions
 *  - call mqtt_register_callback(.)
 *  - the connack_callback will be fired (if supplied)
 *  - you may subscribe to topics using mqtt_construct_subscribe_packet(.)
 *  - the poll_callback will be fired each uip_poll cycle (if supplied)
 *  - the publish_callback will be fired when a publish packet arrives
 *    (if supplied)
 *  - the close_callback will be fired on a connection close/abort
 *    (if supplied)
 *
 * Make sure to only write packets when the connection is established
 * (mqtt_is_connected() returns true), which is guaranteed during the
 * connack, poll, publish callbacks.
 *
 * The MQTT_SENDBUFFER_LENGTH can be configured freely, however do not increase
 * its value above 256. For values >256 the state variables need to be 16 bit.
 *
 * Maybe an explanation of the buffer layout is in order:
 *
 * The mqtt_send_buffer is used for multiple purposes. It may contain (in this
 * order):
 *
 *  - The last sent packet (stored for a uip retransmit request)
 *    Its length is stored in mqtt_send_buffer_last_length (and may be 0)
 *
 *  - The send queue. Binary data that will be sent at the next uip poll
 *    request. mqtt_send_buffer_current_head stores the index behind this data
 *    block, so writing new data to the buffer becomes
 *
 *      mqtt_send_buffer[mqtt_send_buffer_current_head++] = new_byte;
 *
 *  - Unused buffer space.
 *
 *  - Receive buffer. Fragmented packets are stored and will be rebuilt here.
 *    When new data is added the whole receive buffer segment will be copied
 *    the needed amount of bytes backward.
 *
 *
 *  *Limitations*:
 *
 *  - Only one simultaneous connection
 *  - Only one topic per subscription message
 *  - QoS level 0 only (for publish and subscription messages)
 *
 *  Note:
 *
 *  Because of the implementation, the code can actually work with incoming
 *  packets larger than MQTT_SENDBUFFER_LENGTH, but only if they arrive within
 *  one TCP segment.
 *
 */

// TODO check:
// what happens if uip_send() is called after uip_abort()?


#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/bit-macros.h"
#include "protocols/uip/uip.h"
#include "mqtt.h"
#include "mqtt_state.h"
#include "core/debug.h"

// DEBUG MACROS

#ifdef MQTT_DEBUG
#define MQTTDEBUG(...) debug_printf(__VA_ARGS__)
#else
#define MQTTDEBUG(...)
#endif

#ifdef MQTT_PARSE_DEBUG
#define MQTTPARSEDEBUG(...) debug_printf(__VA_ARGS__)
#else
#define MQTTPARSEDEBUG(...)
#endif


#define STATE (&mqtt_con_state)


// BUFFER VARIABLES

static uint8_t mqtt_send_buffer[MQTT_SENDBUFFER_LENGTH];
static uint8_t mqtt_send_buffer_last_length;    // length of last packet
                                              //   (for uip-retransmit)
static uint8_t mqtt_send_buffer_current_head;   // current buffer head
static uint8_t mqtt_receive_buffer_length;      // length of data for received buffer
static uint16_t mqtt_receive_packet_length;     // length of next expected
                                              //   (not fully received) packet

// MQTT PROTOCOL STATE

static uint16_t mqtt_next_msg_id;
static uint16_t mqtt_last_out_activity;
static uint16_t mqtt_last_in_activity;
static bool mqtt_ping_outstanding;

// GENERAL STATE STRUCTURES

static mqtt_connection_config_t const *mqtt_con_config = NULL;
static mqtt_callback_config_t const *mqtt_callbacks[MQTT_CALLBACK_SLOTS];
static mqtt_connection_state_t mqtt_con_state;
static uip_conn_t *mqtt_uip_conn;

static uint16_t mqtt_timer_counter = 0;


/********************
 *                  *
 *   Declarations   *
 *                  *
 ********************/

static inline void mqtt_abort_connection(void);
static inline uint8_t MQTT_LF_LENGTH(uint16_t length);
static inline void mqtt_reset_state(void);
static inline void make_new_message_id(void);
static inline uint16_t minimum(uint16_t a, uint16_t b);

static inline void mqtt_buffer_write_data(const void *data, uint16_t length);
static void mqtt_buffer_write_string(char const *data);
static inline bool mqtt_buffer_free(uint16_t length);
static void mqtt_flush_buffer(void);
static inline void mqtt_retransmit(void);
static inline void mqtt_received_ack(void);
static uint8_t mqtt_buffer_write_length_field(uint8_t * buffer,
                                              uint16_t length);
static bool mqtt_write_to_receive_buffer(const void *data, uint16_t length);

static bool mqtt_construct_connect_packet(void);
bool mqtt_construct_publish_packet(char const *topic, const void *payload,
                                   uint16_t payload_length, bool retain);
bool mqtt_construct_subscribe_packet(char const *topic);
bool mqtt_construct_unsubscribe_packet(char const *topic);
bool mqtt_construct_zerolength_packet(uint8_t msg_type);
bool mqtt_construct_ack_packet(uint8_t msg_type, uint16_t msgid);

static void mqtt_handle_packet(const void *data, uint8_t llen,
                               uint16_t packet_length);
static uint8_t mqtt_parse_length_field(uint16_t * length, const void *buffer,
                                       uint16_t max_read);
static void mqtt_parse(void);

static void mqtt_fire_connack_callback(void);
static void mqtt_fire_poll_callback(void);
static void mqtt_fire_close_callback(void);
static void mqtt_fire_publish_callback(char const *topic,
                                       uint16_t topic_length,
                                       const void *payload,
                                       uint16_t payload_length);

static void mqtt_poll(void);
static void mqtt_main(void);
static void mqtt_init(void);
void mqtt_periodic(void);

bool mqtt_is_connected(void);


/*********************
 *                   *
 *      Misc         *
 *                   *
 *********************/

static inline void
mqtt_abort_connection(void)
{
  uip_abort();
  mqtt_uip_conn = NULL;
  mqtt_fire_close_callback();
}

// return the length of a mqtt variable length field
static inline uint8_t
MQTT_LF_LENGTH(uint16_t length)
{
  if (length < 1 << 8)
    return 1;
  if (length < 1 << 15)
    return 2;
  return 3;                     // more isn't possible with uint16_t
}

// reset state
static inline void
mqtt_reset_state(void)
{
  mqtt_send_buffer_last_length = mqtt_send_buffer_current_head =
    mqtt_receive_buffer_length = mqtt_receive_packet_length = 0;
  mqtt_next_msg_id = 0;
  mqtt_ping_outstanding = false;
  mqtt_last_in_activity = mqtt_last_out_activity = mqtt_timer_counter;
}

// the message id must not be 0
static inline void
make_new_message_id(void)
{
  if (++mqtt_next_msg_id == 0)
    ++mqtt_next_msg_id;
}

// simple mathematics
static inline uint16_t
minimum(uint16_t a, uint16_t b)
{
  return a < b ? a : b;
}


/*********************
 *                   *
 *  Buffer Handling  *
 *                   *
 *********************/

// write data to the buffer (no free space check)
static inline void
mqtt_buffer_write_data(const void *data, uint16_t length)
{
  memcpy(mqtt_send_buffer + mqtt_send_buffer_current_head, data, length);
  mqtt_send_buffer_current_head += length;
}

// write a c-string to the buffer, inserting length field
// (no free space check)
static void
mqtt_buffer_write_string(char const *data)
{
  char const *idp = data;
  mqtt_send_buffer_current_head += 2;

  while (*idp)
    mqtt_send_buffer[mqtt_send_buffer_current_head++] = *idp++;

  uint16_t len = idp - data;

  mqtt_send_buffer[mqtt_send_buffer_current_head - len - 2] = HI8(len);
  mqtt_send_buffer[mqtt_send_buffer_current_head - len - 1] = LO8(len);
}

// return whether the buffer has enough storage room for `length` bytes
static inline bool
mqtt_buffer_free(uint16_t length)
{
  return mqtt_send_buffer_current_head + length + mqtt_receive_buffer_length
    <= MQTT_SENDBUFFER_LENGTH;
}

// flush the send buffer (uip_send) if there is data
static void
mqtt_flush_buffer(void)
{
  if (mqtt_send_buffer_last_length == 0 // no data waiting for a uip_ack
      && mqtt_send_buffer_current_head > 0)
  {
    mqtt_send_buffer_last_length =
      minimum(mqtt_send_buffer_current_head, uip_mss());
    uip_send(mqtt_send_buffer, mqtt_send_buffer_last_length);
    mqtt_last_out_activity = mqtt_timer_counter;
  }
}

// uip wanted a retransmit
static inline void
mqtt_retransmit(void)
{
  if (mqtt_send_buffer_last_length > 0)
    uip_send(mqtt_send_buffer, mqtt_send_buffer_last_length);
}

// respond to a received uip_ack
static inline void
mqtt_received_ack(void)
{
  // discard last sent data
  if (mqtt_send_buffer_last_length > 0)
  {
    // copy queue to beginning of buffer
    memcpy(mqtt_send_buffer, mqtt_send_buffer + mqtt_send_buffer_last_length,
           mqtt_send_buffer_current_head - mqtt_send_buffer_last_length);
    mqtt_send_buffer_current_head -= mqtt_send_buffer_last_length;
    mqtt_send_buffer_last_length = 0;
  }
}

// write mqtt length field to buffer
// return number of bytes written
static uint8_t
mqtt_buffer_write_length_field(uint8_t * buffer, uint16_t length)
{
  uint8_t llen = 0;
  uint8_t digit;
  uint8_t pos = 0;
  do
  {
    digit = length % 128;
    length = length / 128;
    if (length > 0)
    {
      digit |= 0x80;
    }
    buffer[pos++] = digit;
    llen++;
  }
  while (length > 0);
  return llen;
}

// write some data into the receive buffer
// return false if there is not enough free buffer space
static bool
mqtt_write_to_receive_buffer(const void *data, uint16_t length)
{
  if (!mqtt_buffer_free(length))
    return false;

  // move receive buffer backward
  memcpy(mqtt_send_buffer + MQTT_SENDBUFFER_LENGTH -
         mqtt_receive_buffer_length - length,
         mqtt_send_buffer + MQTT_SENDBUFFER_LENGTH -
         mqtt_receive_buffer_length, mqtt_receive_buffer_length);
  // copy new data to the end
  memcpy(mqtt_send_buffer + MQTT_SENDBUFFER_LENGTH - length, data, length);
  // increase data counter
  mqtt_receive_buffer_length += length;

  return true;
}


/*********************
 *                   *
 *  Sending Packets  *
 *                   *
 *********************/

static bool
mqtt_construct_connect_packet(void)
{
  uint8_t protocol_string[9] =
    { 0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTTPROTOCOLVERSION };

  // calculate length
  uint16_t length = sizeof(protocol_string) + 1 // connect flags
    + strlen(mqtt_con_config->client_id) + 2    // client id
    + 2;                        // keep alive
  if (mqtt_con_config->will_topic)
    length += strlen(mqtt_con_config->will_topic)
      + strlen(mqtt_con_config->will_message) + 4;
  if (mqtt_con_config->user)
    length += strlen(mqtt_con_config->user) + 2;
  if (mqtt_con_config->pass)
    length += strlen(mqtt_con_config->pass) + 2;

  // packet length + length field + header flags
  if (!mqtt_buffer_free(length + MQTT_LF_LENGTH(length) + 1))
    return false;               // this should not happen (first paket sent)

  // fixed header
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = MQTTCONNECT;
  mqtt_send_buffer_current_head +=
    mqtt_buffer_write_length_field(mqtt_send_buffer +
                                   mqtt_send_buffer_current_head, length);

  // protocol string
  unsigned int j;
  for (j = 0; j < 9; j++)
  {
    mqtt_send_buffer[mqtt_send_buffer_current_head++] = protocol_string[j];
  }

  // connect flags
  uint8_t connect_flags;
  if (mqtt_con_config->will_topic)
  {
    connect_flags = 0x06
      | (mqtt_con_config->will_qos << 3)
      | ((mqtt_con_config->will_retain ? 1 : 0) << 5);
  }
  else
  {
    connect_flags = 0x02;
  }

  if (mqtt_con_config->user != NULL)
  {
    connect_flags = connect_flags | 0x80;

    if (mqtt_con_config->pass != NULL)
    {
      connect_flags = connect_flags | (0x80 >> 1);
    }
  }

  mqtt_send_buffer[mqtt_send_buffer_current_head++] = connect_flags;

  // keep alive
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = HI8(MQTT_KEEPALIVE);
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = LO8(MQTT_KEEPALIVE);

  // client id
  mqtt_buffer_write_string(mqtt_con_config->client_id);

  // will
  if (mqtt_con_config->will_topic)
  {
    mqtt_buffer_write_string(mqtt_con_config->will_topic);
    mqtt_buffer_write_string(mqtt_con_config->will_message);
  }

  // user / pass
  if (mqtt_con_config->user != NULL)
  {
    mqtt_buffer_write_string(mqtt_con_config->user);
    if (mqtt_con_config->pass != NULL)
    {
      mqtt_buffer_write_string(mqtt_con_config->pass);
    }
  }

  return true;
}


bool
mqtt_construct_publish_packet(char const *topic, const void *payload,
                              uint16_t payload_length, bool retain)
{
  // maybe make this a parameter (at least qos=1 should already be operational)
  const uint8_t qos = 0;

  uint16_t length = strlen(topic) + 2 + payload_length;
  if (qos > 0)
    length += 2;                // message id

  if (!mqtt_buffer_free(length + MQTT_LF_LENGTH(length) + 1))
    return false;

  // header flags
  uint8_t header = MQTTPUBLISH | qos << 1;
  if (retain)
    header |= 1 << 0;

  // fixed header
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = header;
  mqtt_send_buffer_current_head +=
    mqtt_buffer_write_length_field(mqtt_send_buffer +
                                   mqtt_send_buffer_current_head, length);

  // topic
  mqtt_buffer_write_string(topic);

  // message id
  if (qos > 0)
  {
    mqtt_send_buffer[mqtt_send_buffer_current_head++] = HI8(mqtt_next_msg_id);
    mqtt_send_buffer[mqtt_send_buffer_current_head++] = LO8(mqtt_next_msg_id);
    make_new_message_id();
  }

  mqtt_buffer_write_data(payload, payload_length);

  return true;
}


bool
mqtt_construct_subscribe_packet(char const *topic)
{
  uint16_t length = 2           // message id
    + strlen(topic) + 2         // topic
    + 1;                        // qos

  if (!mqtt_buffer_free(length + MQTT_LF_LENGTH(length) + 1))
    return false;

  // fixed header
  const uint8_t qos = 1;
  mqtt_send_buffer[mqtt_send_buffer_current_head++] =
    MQTTSUBSCRIBE | qos << 1;
  mqtt_send_buffer_current_head +=
    mqtt_buffer_write_length_field(mqtt_send_buffer +
                                   mqtt_send_buffer_current_head, length);

  // message id
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = HI8(mqtt_next_msg_id);
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = LO8(mqtt_next_msg_id);
  make_new_message_id();

  // payload: topic + requested qos
  mqtt_buffer_write_string(topic);
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = 0;        // requested qos level

  return true;
}


bool
mqtt_construct_unsubscribe_packet(char const *topic)
{
  uint16_t length = 2           // message id
    + strlen(topic) + 2;        // topic

  if (!mqtt_buffer_free(length + MQTT_LF_LENGTH(length) + 1))
    return false;

  // fixed header
  const uint8_t qos = 1;
  mqtt_send_buffer[mqtt_send_buffer_current_head++] =
    MQTTUNSUBSCRIBE | qos << 1;
  mqtt_send_buffer_current_head +=
    mqtt_buffer_write_length_field(mqtt_send_buffer +
                                   mqtt_send_buffer_current_head, length);

  // message id
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = HI8(mqtt_next_msg_id);
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = LO8(mqtt_next_msg_id);
  make_new_message_id();

  // payload: topic
  mqtt_buffer_write_string(topic);

  return true;
}


// msg_type may be one of:
// MQTTPINGREQ
// MQTTPINGRESP
// MQTTDISCONNECT
bool
mqtt_construct_zerolength_packet(uint8_t msg_type)
{
  if (!mqtt_buffer_free(2))
    return false;

  // fixed header
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = msg_type;
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = 0;        // length field

  return true;
}


// msg_type may be one of:
// MQTTPUBACK
// MQTTPUBREC
// MQTTPUBREL // qos of 1 is automatically added
// MQTTPUBCOMP
//
// msgid is the id of the message being ack'ed
bool
mqtt_construct_ack_packet(uint8_t msg_type, uint16_t msgid)
{
  if (!mqtt_buffer_free(4))
    return false;

  uint8_t header_flags = msg_type;
  if (msg_type == MQTTPUBREL)
    header_flags |= MQTTQOS1;

  // fixed header
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = header_flags;
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = 2;        // length field

  // message id
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = HI8(msgid);
  mqtt_send_buffer[mqtt_send_buffer_current_head++] = LO8(msgid);

  return true;
}


/***********************
 *                     *
 *  Receiving Packets  *
 *                     *
 ***********************/

// parse and react to a received packet
static void
mqtt_handle_packet(const void *data, uint8_t llen, uint16_t packet_length)
{
  mqtt_last_in_activity = mqtt_timer_counter;

  const uint8_t *packet = data + llen + 1;
  uint8_t header = *(uint8_t *) data;


  //
  // STATE: CONNECT
  //

  if (STATE->stage == MQTT_STATE_CONNECT)
  {
    // only accept connack packets
    if ((header & 0xf0) != MQTTCONNACK)
    {
      MQTTDEBUG("expected connack packet, aborting\n");
      mqtt_abort_connection();
      return;
    }

    // assert packet length
    if (packet_length < 2)
    {
      MQTTDEBUG("packet length assert connack, aborting\n");
      mqtt_abort_connection();
      return;
    }

    // check return code
    if (packet[1] != 0)
    {
      MQTTDEBUG("connection request error code, aborting\n");
      mqtt_abort_connection();
      return;
    }

    MQTTDEBUG("connack received\n");
    STATE->stage = MQTT_STATE_CONNECTED;

    // auto subscribe
    if (mqtt_con_config->auto_subscribe_topics)
      for (uint8_t i = 0; mqtt_con_config->auto_subscribe_topics[i] != NULL;
           i++)
        mqtt_construct_subscribe_packet(mqtt_con_config->
                                        auto_subscribe_topics[i]);

    mqtt_fire_connack_callback();
  }


  //
  // STATE: CONNECTED
  //

  else if (STATE->stage == MQTT_STATE_CONNECTED)
  {
    switch (header & 0xf0)
    {
      case MQTTPUBLISH:

        ;                       // a declaration can't be the first statement after a case label
        uint8_t qos = (header & 0x06) >> 1;

        // assert packet length
        if (packet_length < 2 + (qos ? 2 : 0))
        {
          MQTTDEBUG("packet length assert pub1, aborting\n");
          mqtt_abort_connection();
          return;
        }

        uint16_t topic_length = packet[0] * 256 + packet[1];

        // assert packet length again
        if (packet_length < 2 + (qos ? 2 : 0) + topic_length)
        {
          MQTTDEBUG("packet length assert pub2, aborting\n");
          mqtt_abort_connection();
          return;
        }

        // calculate packet length for publish callback
        const void *payload = packet + 2 + topic_length;
        uint16_t payload_length = packet_length - (2 + topic_length);

        if (qos > 0)
        {
          payload += 2;
          payload_length -= 2;
        }

        mqtt_fire_publish_callback((char *) packet + 2, topic_length,
                                   payload, payload_length);

        // check for qos level, send ack
        if (qos > 0)
        {
          uint16_t msgid = packet[2 + topic_length] * 256
            + packet[2 + topic_length + 1];

          if (qos == 1)
            mqtt_construct_ack_packet(MQTTPUBACK, msgid);

          else if (qos == 2)
            mqtt_construct_ack_packet(MQTTPUBREC, msgid);
        }

        MQTTDEBUG("publish received\n");

        break;


      case MQTTPUBACK:
        break;                  // hmm 'kay


      case MQTTPUBREC:

        // assert packet length
        if (packet_length < 2)
        {
          MQTTDEBUG("packet length assert rec, aborting\n");
          mqtt_abort_connection();
          return;
        }

        {
          uint16_t msgid = packet[0] * 256 + packet[1];

          mqtt_construct_ack_packet(MQTTPUBREL, msgid);
        }

        break;


      case MQTTPUBREL:

        // assert packet length
        if (packet_length < 2)
        {
          MQTTDEBUG("packet length assert rel, aborting\n");
          mqtt_abort_connection();
          return;
        }

        {
          uint16_t msgid = packet[0] * 256 + packet[1];

          mqtt_construct_ack_packet(MQTTPUBCOMP, msgid);
        }

        break;


      case MQTTPUBCOMP:
        break;                  // hmm 'kay


      case MQTTSUBACK:
      case MQTTUNSUBACK:
        break;                  // hmm 'kay


      case MQTTPINGREQ:
        MQTTDEBUG("pingreq received\n");
        mqtt_construct_zerolength_packet(MQTTPINGRESP);
        break;


      case MQTTPINGRESP:
        MQTTDEBUG("pingresp received\n");
        mqtt_ping_outstanding = false;
        break;


      case MQTTCONNACK:
      case MQTTCONNECT:
      case MQTTSUBSCRIBE:
      case MQTTUNSUBSCRIBE:
      case MQTTDISCONNECT:
        break;                  // ?
    }
  }


  //
  // STATE: UNKNOWN
  //

  else
  {
    MQTTDEBUG("unkown state\n");        // uhmm
  }
}

// parse the variable length field
// return number of bytes read
static uint8_t
mqtt_parse_length_field(uint16_t * length, const void *buffer,
                        uint16_t max_read)
{
  uint16_t l = 0;

  for (uint8_t i = 0; i < max_read; i++)
  {
    l += (((uint8_t *) buffer)[i] & (~0x80)) << (i * 7);
    if ((((uint8_t *) buffer)[i] & 0x80) == 0x00)
    {
      *length = l;
      return i + 1;
    }
  }

  return 0;                     // length field is not complete
}

// parse the input buffer/stream, rebuild fragmented packets, delegate
// handle_packet when a full packet is received
static void
mqtt_parse(void)
{
  // Approach:
  //
  // Parse packets, one at a time, until no more data is left. If the whole
  // packet is available at once, it will be passed on right from the incoming
  // buffer. Otherwise, when the packet is fragmented, two situations might
  // arise.
  //
  // First if the variable length field (used to determine the size of the MQTT
  // packet) is completely received, all received data will be copied into the
  // receive buffer (see buffer layout, receive buffer refers to a part of
  // `mqtt_send_buffer`) and the expected length of the packet is stored in
  // `mqtt_receive_packet_length`.
  //
  // Second, if the variable length field is not complete, the available data
  // is copied to the recieve buffer, too, but `mqtt_receive_packet_length` will be
  // zero, indicating that the variable length field has yet to be parsed.
  //
  // If there is data from a fragmented packet (indicated by the receive buffer
  // being non-empty) the data will be concatenated, the variable length field
  // parsed (if still needed), and eventually a completed packet will be passed
  // on.
  //
  // *Note*:
  //
  // In any error condition (not enough buffer space, inconsistent length
  // field, etc.) the connection will be aborted.


  uint16_t remaining_length;
  uint16_t bytes_read = 0;

  while (bytes_read < uip_len)
  {
    MQTTPARSEDEBUG("loop\n");
    remaining_length = uip_len - bytes_read;

    // no data in receive buffer
    if (!mqtt_receive_buffer_length)
    {
      MQTTPARSEDEBUG("no_buffer\n");
      // try to parse the length field
      uint16_t packet_length = 0;
      // the +1 and -1 come from the first header byte
      uint8_t llen = mqtt_parse_length_field(&packet_length,
                                             uip_appdata + bytes_read + 1,
                                             remaining_length - 1);

      // length field is not completely received
      if (llen == 0)
      {
        MQTTPARSEDEBUG("length field incomplete\n");
        if (!mqtt_write_to_receive_buffer(uip_appdata + bytes_read,
                                          remaining_length))
        {
          MQTTPARSEDEBUG("buffer_full1\n");
          mqtt_abort_connection();
          return;
        }
        bytes_read += remaining_length;
      }

      // the complete packet is in the uip buffer
      else if (packet_length + llen + 1 <= remaining_length)
      {
        MQTTPARSEDEBUG("handle_packet\n");
        mqtt_handle_packet(uip_appdata + bytes_read, llen, packet_length);
        bytes_read += packet_length + llen + 1;
      }

      // length field received, packet incomplete
      // write it to receive buffer
      else
      {
        mqtt_receive_packet_length = packet_length;
        if (!mqtt_write_to_receive_buffer(uip_appdata + bytes_read,
                                          remaining_length))
        {
          MQTTPARSEDEBUG("buffer_full2\n");
          mqtt_abort_connection();
          return;
        }
        bytes_read += remaining_length;
      }
    }


    // receive buffer non-empty, there is data of a fragmented packet
    // packet length already known
    else if (mqtt_receive_packet_length != 0)
    {
      MQTTPARSEDEBUG("length_known\n");
      // check whether we now have the complete packet
      if (mqtt_receive_buffer_length + remaining_length >=
          mqtt_receive_packet_length)
      {
        uint16_t fragment_length = mqtt_receive_packet_length
          - mqtt_receive_buffer_length;

        // write this last fragment to the receive buffer
        if (!mqtt_write_to_receive_buffer(uip_appdata + bytes_read,
                                          fragment_length))
        {
          MQTTPARSEDEBUG("buffer_full3\n");
          mqtt_abort_connection();
          return;
        }

        mqtt_handle_packet(mqtt_send_buffer + MQTT_SENDBUFFER_LENGTH -
                           mqtt_receive_buffer_length,
                           MQTT_LF_LENGTH(mqtt_receive_packet_length),
                           mqtt_receive_packet_length);
        bytes_read += fragment_length;
      }

      else
      {
        // the packet is still incomplete

        // write this new fragment to the receive buffer
        if (!mqtt_write_to_receive_buffer(uip_appdata + bytes_read,
                                          remaining_length))
        {
          MQTTPARSEDEBUG("buffer_full4\n");
          mqtt_abort_connection();
          return;
        }
        bytes_read += remaining_length;
      }
    }


    // receive buffer non-empty, there is data of a fragmented packet
    // packet length not known, try to parse variable length field
    else
    {
      MQTTPARSEDEBUG("length_unknown\n");
      // write one byte to the receive buffer, and try to parse the length
      // field again
      if (!mqtt_write_to_receive_buffer(uip_appdata + bytes_read, 1))
      {
        MQTTPARSEDEBUG("buffer_full5\n");
        mqtt_abort_connection();
        return;
      }
      bytes_read += 1;

      // try to parse the length field
      uint16_t packet_length = 0;
      // the +1 and -1 come from the first header byte
      uint8_t llen = mqtt_parse_length_field(&packet_length,
                                             mqtt_send_buffer +
                                             MQTT_SENDBUFFER_LENGTH -
                                             mqtt_receive_buffer_length + 1,
                                             mqtt_receive_buffer_length - 1);

      if (llen != 0)
        mqtt_receive_packet_length = packet_length;

      // Now just fall through the loop.
      //
      // This will land in the 'fragmented packet with known length'-case, if
      // the parsing was successful.
      //
      // Otherwise this will land in this case here again, and the parsing of
      // the variable length field will be tried with an additional byte.
      //
      // This approach will also silently wait for new data, if the incoming
      // buffer has been read completely.
    }
  }
}


/********************
 *                  *
 *    Callbacks     *
 *                  *
 ********************/

static void
mqtt_fire_connack_callback(void)
{
  for (int i = 0; i < MQTT_CALLBACK_SLOTS; ++i)
  {
    if (mqtt_callbacks[i] != NULL)
    {
      connack_callback cb =
        (connack_callback) pgm_read_word(&mqtt_callbacks[i]->
                                         connack_callback);
      if (cb != NULL)
        cb();
    }
  }
}

static void
mqtt_fire_poll_callback(void)
{
  for (int i = 0; i < MQTT_CALLBACK_SLOTS; ++i)
  {
    if (mqtt_callbacks[i] != NULL)
    {
      poll_callback cb =
        (poll_callback) pgm_read_word(&mqtt_callbacks[i]->poll_callback);
      if (cb != NULL)
        cb();
    }
  }
}

static void
mqtt_fire_close_callback(void)
{
  for (int i = 0; i < MQTT_CALLBACK_SLOTS; ++i)
  {
    if (mqtt_callbacks[i] != NULL)
    {
      close_callback cb =
        (close_callback) pgm_read_word(&mqtt_callbacks[i]->close_callback);
      if (cb != NULL)
        cb();
    }
  }
}

static void
mqtt_fire_publish_callback(char const *topic, uint16_t topic_length,
                           const void *payload, uint16_t payload_length)
{
  for (int i = 0; i < MQTT_CALLBACK_SLOTS; ++i)
  {
    if (mqtt_callbacks[i] != NULL)
    {
      publish_callback cb =
        (publish_callback) pgm_read_word(&mqtt_callbacks[i]->
                                         publish_callback);
      if (cb != NULL)
        cb(topic, topic_length, payload, payload_length);
    }
  }
}


/********************
 *                  *
 *    Main Logic    *
 *                  *
 ********************/

// periodically called function, detect and react to timeouts
// (only called while there is a valid uip connection)
static void
mqtt_poll(void)
{
  if ((mqtt_timer_counter - mqtt_last_in_activity >
       MQTT_KEEPALIVE * TIMER_TICKS_PER_SECOND) ||
      (mqtt_timer_counter - mqtt_last_out_activity >
       MQTT_KEEPALIVE * TIMER_TICKS_PER_SECOND))
  {
    if (mqtt_ping_outstanding)
    {
      MQTTDEBUG("missed ping, aborting\n");
      mqtt_abort_connection();
      return;
    }
    else
    {
      MQTTDEBUG("ping request\n");
      mqtt_construct_zerolength_packet(MQTTPINGREQ);

      // reset counter, wait another keepalive period
      mqtt_last_in_activity = mqtt_timer_counter;
      mqtt_ping_outstanding = true;
    }
  }

  mqtt_fire_poll_callback();
}

// mqtt uip callback, manage connection state, delegate I/O
static void
mqtt_main(void)
{

  if (uip_aborted() || uip_timedout())
  {
    MQTTDEBUG("connection aborted\n");
    mqtt_uip_conn = NULL;

    mqtt_fire_close_callback();

    return;
  }

  if (uip_closed())
  {
    MQTTDEBUG("connection closed\n");
    mqtt_uip_conn = NULL;
    return;
  }

  if (uip_connected())
  {
    MQTTDEBUG("new connection\n");

    mqtt_construct_connect_packet();

    // init
    mqtt_next_msg_id = 1;
    STATE->stage = MQTT_STATE_CONNECT;

    // send
    mqtt_flush_buffer();
  }

  if (uip_acked())
  {
    MQTTDEBUG("acked\n");
    mqtt_received_ack();
  }

  if (uip_rexmit())
  {
    MQTTDEBUG("mqtt main rexmit\n");
    mqtt_retransmit();
  }

  else if (uip_newdata() && uip_len)
  {
    MQTTDEBUG("received data: \n");
    mqtt_parse();
  }

  else if (uip_poll() && STATE->stage == MQTT_STATE_CONNECTED)
  {
    MQTTDEBUG("mqtt main poll\n");
    mqtt_poll();
    mqtt_flush_buffer();
  }

  else if (uip_poll() && STATE->stage == MQTT_STATE_CONNECT)
  {
    if (mqtt_timer_counter - mqtt_last_in_activity
        > MQTT_KEEPALIVE * TIMER_TICKS_PER_SECOND)
    {
      MQTTDEBUG("connect request timed out\n");
      mqtt_abort_connection();
      return;
    }
  }
}


// periodic mqtt timer callback, initialize connection if non-active
void
mqtt_periodic(void)
{
  mqtt_timer_counter++;

  if (!mqtt_uip_conn)
    mqtt_init();
}


// initialize mqtt connection if config data has been supplied
static void
mqtt_init(void)
{
  if (!mqtt_uip_conn)
  {
    if (!mqtt_con_config)
    {
      MQTTDEBUG("cannot initialize mqtt client, no config\n");
      return;
    }

    MQTTDEBUG("initializing mqtt client\n");

    mqtt_reset_state();         // reset state
    // uip_connect wants a non-constant pointer
    mqtt_uip_conn =
      uip_connect((uip_ipaddr_t *) & mqtt_con_config->target_ip, HTONS(1883),
                  mqtt_main);

    if (!mqtt_uip_conn)
    {
      MQTTDEBUG("no uip_conn available.\n");
      return;
    }
  }
}


// Set the connection config for mqtt
void
mqtt_set_connection_config(mqtt_connection_config_t const *const config)
{
  mqtt_con_config = config;
}

// Register a set of callbacks
// Return the assigned slot id, which can be used to unregister the
// callbacks later. Return 0xff if there is no free slot.
uint8_t
mqtt_register_callback(mqtt_callback_config_t const *const callbacks)
{
  // search a free slot
  for (uint8_t i = 0; i < MQTT_CALLBACK_SLOTS; ++i)
    if (mqtt_callbacks[i] == NULL)
    {
      mqtt_callbacks[i] = callbacks;
      return i;
    }
  return 0xff;
}

// Unregister a set of callbacks using the previously returned slot id
void
mqtt_unregister_callback(uint8_t slot_id)
{
  if (slot_id >= MQTT_CALLBACK_SLOTS)
    return;

  mqtt_callbacks[slot_id] = NULL;
}

bool
mqtt_is_connected(void)
{
  return (bool) mqtt_uip_conn;
}

/*
  -- Ethersex META --
  header(protocols/mqtt/mqtt.h)
  timer(1, mqtt_periodic())
*/
