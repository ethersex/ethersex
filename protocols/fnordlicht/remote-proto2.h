/*
 *
 * Copyright (c) 2010 by Stefan Riepenhausen <rhn@gmx.net>
 * 
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __REMOTE_PROTO
#define __REMOTE_PROTO 1

#include "remote-proto.h"
#include "color.h"
//#include "static_programs.h"

#define FNORDLICHT_PROGRAM_PARAMETER_SIZE 10

struct remote_msg_fade_rgb_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct rgb_color_t color;
};

struct remote_msg_fade_hsv_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct hsv_color_t color;
};

struct remote_msg_save_rgb_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
    struct rgb_color_t color;
};

struct remote_msg_save_hsv_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
    struct hsv_color_t color;
};

struct remote_msg_save_current_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
};

struct remote_msg_config_offsets_t
{
    uint8_t address;
    uint8_t cmd;
    int8_t step;
    int8_t delay;
    int16_t hue;
    uint8_t saturation;
    uint8_t value;
};
/*
struct remote_msg_start_program_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t script;
    union program_params_t params;
};
*/
struct remote_msg_stop_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t fade;
};

struct remote_msg_modify_current_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct rgb_color_offset_t rgb;
    struct hsv_color_offset_t hsv;
};

struct remote_msg_pull_int_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t delay;
};

enum startup_mode_t
{
    FNORDLICHT_STARTUP_NOTHING = 0,
    FNORDLICHT_STARTUP_PROGRAM = 1,
};

/* startup parameters including mode, size: 12 byte */
struct startup_parameters_t
{
    enum startup_mode_t mode;

    union {
        /* raw access to data, size: 11 byte */
        uint8_t raw[11];

        /* structure for startup_mode == STARTUP_PROGRAM
         * size: 11 byte */
        struct {
            uint8_t program;
            uint8_t program_parameters[FNORDLICHT_PROGRAM_PARAMETER_SIZE];
        };
    };
};

struct remote_msg_config_startup_t
{
    uint8_t address;
    uint8_t cmd;
    struct startup_parameters_t params;
};

#endif
