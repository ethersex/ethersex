/*
 *
 * Copyright (c) 2015 by Daniel Lindner <daniel.lindner@gmx.de>
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
#include "core/debug.h"
#include "core/bit-macros.h"
#include "core/util/fixedpoint.h"
#include "core/util/string_parsing.h"
#include "protocols/bsbport/bsbport.h"
#include "protocols/bsbport/bsbport_tx.h"
#include "protocols/mqtt/mqtt.h"

#define BSBPORT_PUBLISH_FORMAT          BSBPORT_MQTT_TOPIC "/%02x/%02x%02x%02x%02x/"
#define BSBPORT_SUBSCRIBE_SET_TOPIC     BSBPORT_MQTT_TOPIC "/set/#"
#define BSBPORT_SUBSCRIBE_QUERY_TOPIC   BSBPORT_MQTT_TOPIC "/query/#"
#define BSBPORT_SUBSCRIBE_SET_FORMAT    BSBPORT_MQTT_TOPIC "/set/%2hhx/%2hhx%2hhx%2hhx%2hhx/%3s"
#define BSBPORT_SUBSCRIBE_QUERY_FORMAT  BSBPORT_MQTT_TOPIC "/query/%2hhx/%2hhx%2hhx%2hhx%2hhx"
#define BSBPORT_MQTT_RETAIN             false
#define DATA_LENGTH                     8
#define TOPIC_LENGTH                    (sizeof(BSBPORT_MQTT_TOPIC) + 17)

static const char publish_topic_format[] PROGMEM = BSBPORT_PUBLISH_FORMAT;

#ifdef DEBUG_BSBPORT_MQTT
#define BSBDEBUG(s, ...) debug_printf("BSB " s "\n", ## __VA_ARGS__)
#else
#define BSBDEBUG(...) do { } while(0)
#endif

void
bsbport_poll_cb(void)
{
  BSBDEBUG("MQTT Poll");
  for (uint8_t i = 0; i < BSBPORT_MESSAGE_BUFFER_LEN; i++)
  {
    if (bsbport_msg_buffer.msg[i].mqtt_new)
    {
      uint8_t len;
      uint8_t topic_len;
      char buf[DATA_LENGTH];
      char topic[TOPIC_LENGTH];

      bsbport_msg_buffer.msg[i].mqtt_new = 0;

      // Topic
      topic_len = snprintf_P(topic, TOPIC_LENGTH, publish_topic_format,
                 LO4(bsbport_msg_buffer.msg[i].src),
                 bsbport_msg_buffer.msg[i].p.data.p1,
                 bsbport_msg_buffer.msg[i].p.data.p2,
                 bsbport_msg_buffer.msg[i].p.data.p3,
                 bsbport_msg_buffer.msg[i].p.data.p4);

      // RAW
      strncat_P(topic, PSTR("RAW"), TOPIC_LENGTH - topic_len - 1); 
      len =
        snprintf_P(buf, DATA_LENGTH, PSTR("%u"),
                   bsbport_msg_buffer.msg[i].value);
      mqtt_construct_publish_packet(topic, buf, len, BSBPORT_MQTT_RETAIN);
      BSBDEBUG("%s=%s", topic, buf);

      // STA
      topic[topic_len]=0;
      strncat_P(topic, PSTR("STA"), TOPIC_LENGTH - topic_len - 1); 
      len =
        snprintf_P(buf, DATA_LENGTH, PSTR("%u"),
                   HI8(bsbport_msg_buffer.msg[i].value));
      mqtt_construct_publish_packet(topic, buf, len, BSBPORT_MQTT_RETAIN);
      BSBDEBUG("%s=%s", topic, buf);

      // TMP
      topic[topic_len]=0;
      strncat_P(topic, PSTR("TMP"), TOPIC_LENGTH - topic_len - 1); 
      len =
        itoa_fixedpoint(((int32_t) bsbport_msg_buffer.msg[i].value * 100) /
                        64, 2, buf, DATA_LENGTH);
      mqtt_construct_publish_packet(topic, buf, len, BSBPORT_MQTT_RETAIN);
      BSBDEBUG("%s=%s", topic, buf);

      // FP1
      topic[topic_len]=0;
      strncat_P(topic, PSTR("FP1"), TOPIC_LENGTH - topic_len - 1); 
      len =
        itoa_fixedpoint(bsbport_msg_buffer.msg[i].value, 1, buf, DATA_LENGTH);
      mqtt_construct_publish_packet(topic, buf, len, BSBPORT_MQTT_RETAIN);
      BSBDEBUG("%s=%s", topic, buf);

      // FP5
      topic[topic_len]=0;
      strncat_P(topic, PSTR("FP5"), TOPIC_LENGTH - topic_len - 1); 
      len =
        itoa_fixedpoint(bsbport_msg_buffer.msg[i].value * 10 / 2, 1, buf,
                        DATA_LENGTH);
      mqtt_construct_publish_packet(topic, buf, len, BSBPORT_MQTT_RETAIN);
      BSBDEBUG("%s=%s", topic, buf);
    }
  }
}

void
bsbport_publish_cb(char const *topic, uint16_t topic_length,
                   const void *payload, uint16_t payload_length)
{
  BSBDEBUG("MQTT Publish: %s", topic);
  if (topic_length < 20)
    return;
  uint8_t p1;
  uint8_t p2;
  uint8_t p3;
  uint8_t p4;
  uint8_t dest = 0;
  uint8_t ret;
  char type[4];
  int16_t fp_val = 0;
  uint16_t raw_val = 0;

  if (topic[sizeof(BSBPORT_MQTT_TOPIC)] == 's')
  {
    char *strvalue = malloc(payload_length + 1);
    ret =
      sscanf_P(topic, PSTR(BSBPORT_SUBSCRIBE_SET_FORMAT), &dest, &p1, &p2,
               &p3, &p4, type);
    memcpy(strvalue, payload, payload_length);
    strvalue[payload_length] = 0;
    sscanf_P(strvalue, PSTR("%i"), &raw_val);
    next_int16_fp(strvalue, &fp_val, 1);

    BSBDEBUG("MQTT set ARGS:%d %02x %02x %02x %02x %02x %3s %s %d %u", ret,
             p1, p2, p3, p4, dest, type, strvalue, fp_val, raw_val);

    if (ret == 6)
    {
      uint8_t data[3];
      uint8_t datalen = 3;
      data[0] = 0x01;
      if (type[0] == 'R') //set RAW value
      {
        data[1] = HI8(raw_val);
        data[2] = LO8(raw_val);
      }
      else if (type[0] == 'S') //set SELECT/SEL value
      {
        data[1] = LO8(raw_val);
        datalen = 2;
      }
      else if (type[0] == 'T') //set TEMP/TMP value
      {
        int16_t tmp;
        tmp = ((int32_t) fp_val * 64) / 10;
        data[1] = HI8(tmp);
        data[2] = LO8(tmp);
      }
      else if (type[0] == 'F') //set FP1/FP5 value
      {
        if (type[2] == '1') //set FP1 value
        {
          data[1] = HI8(fp_val);
          data[2] = LO8(fp_val);
        }
        else if (type[2] == '5') //set FP1 value
        {
          int16_t tmp;
          tmp = fp_val * 2 / 10;
          data[1] = HI8(tmp);
          data[2] = LO8(tmp);
        }
      }
      else
      {
        BSBDEBUG("MQTT set type unkown");
        return;
      }
      BSBDEBUG("MQTT set send: %02x %02x %02x %02x %02x %02x %02x %02x %d",
               p1, p2, p3, p4, dest, data[0], data[1], data[2], datalen);
      bsbport_set(p1, p2, p3, p4, dest, data, datalen);
    }
    else
      BSBDEBUG("MQTT set parse error");
  }
  if (topic[sizeof(BSBPORT_MQTT_TOPIC)] == 'q')
  {
    ret =
      sscanf_P(topic, PSTR(BSBPORT_SUBSCRIBE_QUERY_FORMAT), &dest, &p1, &p2,
               &p3, &p4);
    if (ret == 5)
    {
      BSBDEBUG("MQTT Query send: %02x %02x %02x %02x %02x", p1, p2, p3, p4,
               dest);
      bsbport_query(p1, p2, p3, p4, dest);
    }
    else
      BSBDEBUG("MQTT query parse error");
  }
  else
    BSBDEBUG("MQTT parse error");
}

static void
bsbport_connack_cb(void)
{
  BSBDEBUG("MQTT Sub: " BSBPORT_SUBSCRIBE_SET_TOPIC);
  mqtt_construct_subscribe_packet(BSBPORT_SUBSCRIBE_SET_TOPIC);
  BSBDEBUG("MQTT Sub: " BSBPORT_SUBSCRIBE_QUERY_TOPIC);
  mqtt_construct_subscribe_packet(BSBPORT_SUBSCRIBE_QUERY_TOPIC);
}

static const mqtt_callback_config_t mqtt_callback_config PROGMEM = {
  .connack_callback = bsbport_connack_cb,
  .poll_callback = bsbport_poll_cb,
  .close_callback = NULL,
  .publish_callback = bsbport_publish_cb,
};

void
bsbport_mqtt_init()
{
  BSBDEBUG("MQTT Init");
  mqtt_register_callback(&mqtt_callback_config);
}

/*
  -- Ethersex META --
  header(protocols/bsbport/bsbport_mqtt.h)
  net_init(bsbport_mqtt_init)
*/
