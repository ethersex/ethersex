/*
 * Copyright (c) 2014 by Philip Matura <ike@tura-home.de>
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
 * This file enables static configuration for MQTT through make menuconfig. If
 * this feature ist not selected, this file is processed to be empty.
 */


#include "config.h"
#include "mqtt.h"


#ifdef MQTT_STATIC_CONF

#ifndef MQTT_STATIC_CONF_SUPPLY_USERNAME
  #define MQTT_STATIC_CONF_USER NULL
  #define MQTT_STATIC_CONF_PASS NULL
#else
  #ifndef MQTT_STATIC_CONF_SUPPLY_PASSWORD
    #define MQTT_STATIC_CONF_PASS NULL
  #endif
#endif

#ifndef MQTT_STATIC_CONF_SUPPLY_WILL
  #define MQTT_STATIC_CONF_WILL_TOPIC NULL
  #define MQTT_STATIC_CONF_WILL_QOS 0
  #define MQTT_STATIC_CONF_WILL_RETAIN_FLAG false
  #define MQTT_STATIC_CONF_WILL_MESSAGE NULL
#else
  #ifdef MQTT_STATIC_CONF_WILL_RETAIN
    #define MQTT_STATIC_CONF_WILL_RETAIN_FLAG true
  #else
    #define MQTT_STATIC_CONF_WILL_RETAIN_FLAG false
  #endif
#endif

static char const *mqtt_static_conf_autosubscribe[] = {NULL};
static mqtt_connection_config_t mqtt_static_conf =
{
  .client_id = MQTT_STATIC_CONF_CLIENTID,
  .user = MQTT_STATIC_CONF_USER,
  .pass = MQTT_STATIC_CONF_PASS,
  .will_topic = MQTT_STATIC_CONF_WILL_TOPIC,
  .will_qos = MQTT_STATIC_CONF_WILL_QOS,
  .will_retain = MQTT_STATIC_CONF_WILL_RETAIN_FLAG,
  .will_message = MQTT_STATIC_CONF_WILL_MESSAGE,
  .auto_subscribe_topics = mqtt_static_conf_autosubscribe,
  //target_ip not set, see mqtt_set_static_conf
};

void
mqtt_set_static_conf(void)
{
  set_MQTT_STATIC_CONF_SERVER_IP(&mqtt_static_conf.target_ip);
  mqtt_set_connection_config(&mqtt_static_conf);
}

#endif

/*
  -- Ethersex META --
  initearly(mqtt_set_static_conf)
*/
