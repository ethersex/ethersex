/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
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

//#include <stdint.h>

#ifndef COLOR_H
#define COLOR_H


struct rgb_color_t
{
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
    };
};

struct rgb_color_offset_t {
    union {
        struct {
            int8_t red;
            int8_t green;
            int8_t blue;
        };
        int8_t rgb[3];
    };
};

struct hsv_color_t
{
    union {
        struct {
            uint16_t hue;
            uint8_t saturation;
            uint8_t value;
        };
        uint8_t hsv[4];
    };
};

struct hsv_color_offset_t {
    int16_t hue;
    int8_t saturation;
    int8_t value;
};

struct dual_color_t
{
    struct rgb_color_t rgb;
    struct hsv_color_t hsv;
};

union color_t
{
    /* rgb */
    struct {
        union {
            struct {
                uint8_t red;
                uint8_t green;
                uint8_t blue;
            };
            uint8_t rgb[3];
        };
        /* marker, 0xff if rgb */
        uint8_t rgb_marker;
    };

    /* hsv */
    struct {
        uint8_t saturation;
        uint8_t value;
        /* 0 <= hue <= 349, otherwise rgb might be assumed */
        uint16_t hue;
    };
};

#endif
