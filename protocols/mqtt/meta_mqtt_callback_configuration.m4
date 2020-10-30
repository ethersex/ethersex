dnl
dnl   Copyright (c) 2020 by Erik Kunze <ethersex@erik-kunze.de>
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License as published by
dnl   the Free Software Foundation; either version 3 of the License, or
dnl   (at your option) any later version.
dnl
dnl
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public License
dnl   along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl
dnl -------------------------
divert(-1)dnl
dnl debugfile(`meta_mqtt_configuration.trace')dnl
dnl debugmode(`V')dnl

dnl save current divert value
define(`pushdivert', `define(`_old_divert', divnum)')dnl
define(`popdivert', `divert(_old_divert)')dnl

dnl diverts used in this script
define(`preamble_divert',0)dnl
define(`prototype_divert', `eval(preamble_divert` + 1')')dnl
define(`module_config_divert', `eval(prototype_divert` + 1')')dnl
define(`postamble_divert', `eval(module_config_divert` + 1')')dnl

dnl mqtt_conf(module_config)
dnl the first argument specifies the mqtt topic which subscribes to
dnl the module, the second argument specifies the callback
define(`mqtt_conf', `dnl
pushdivert()dnl
divert(prototype_divert)extern const mqtt_callback_config_t $1;
divert(module_config_divert)  &$1,
popdivert()')
divert(preamble_divert)dnl
/*
 * This file has been generated automatically by meta_mqtt_configuration.m4.
 * Please do not modify it, edit the m4 scripts instead.
 */

#include "protocols/mqtt/mqtt.h"

/*
 * Prototypes
 */
divert(prototype_divert)dnl
divert(module_config_divert)dnl


/*
 * All the static mqtt module configs
 */
const mqtt_callback_config_t *const mqtt_static_callbacks[] PROGMEM = {
divert(postamble_divert)dnl

};

const uint8_t mqtt_static_callback_slots = sizeof(mqtt_static_callbacks)/sizeof(mqtt_static_callbacks[0]);

divert(-1)dnl
