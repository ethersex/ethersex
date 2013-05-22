/*
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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

#include <string.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "protocols/sgc/sgc.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_sgc_set_powerstate(char *cmd, char *output, uint16_t len)
{
  uint8_t power_soll;
  if ((sscanf_P(cmd, PSTR("%hhu"), &power_soll) != 1) ||
      ((power_soll != 0) && (power_soll != 1)))
    return ECMD_ERR_PARSE_ERROR;
  if (sgc_setpowerstate(power_soll) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t
parse_cmd_sgc_get_powerstate(char *cmd, char *output, uint16_t len)
{
  uint8_t pwrstate;
  pwrstate = sgc_getpowerstate();
  if (pwrstate == SHUTDOWN)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("SHUTDOWN")));
  if (pwrstate == POWERUP)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("POWERUP")));
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

#ifdef SGC_ECMD_SEND_SUPPORT
int16_t
parse_cmd_sgc_set_ip(char *cmd, char *output, uint16_t len)
{
  if (sgc_setip(cmd) == 0)
    return ECMD_FINAL_OK;
  return ECMD_ERR_PARSE_ERROR;
}
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT

int16_t
parse_cmd_sgc_set_timeout(char *cmd, char *output, uint16_t len)
{
  uint8_t time;
  if ((sscanf_P(cmd, PSTR("%hhu"), &time) != 1) || (time == 0)) /* 1...255 minutes valid */
    return ECMD_ERR_PARSE_ERROR;
  sgc_settimeout(time);

  return ECMD_FINAL_OK;
}

#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

int16_t
parse_cmd_sgc_result(char *cmd, char *output, uint16_t len)
{
  uint8_t cmdres;
  cmdres = sgc_getcommandresult();
  switch (cmdres)
  {
    case FROM_RESET:           /* 0 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("RESET")));
    case ACK:                  /* 1 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("ACK")));
    case NACK:                 /* 2 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("NACK")));
    case SENDING:              /* 3 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("SENDING")));
    case NONE:                 /* 4 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("NONE")));
    default:                   /* 5 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
  }
}

int16_t
parse_cmd_sgc_contrast(char *cmd, char *output, uint16_t len)
{
  uint8_t contrast;

  if ((sscanf_P(cmd, PSTR("%hhu"), &contrast) != 1) || (contrast > 0x0F))
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_setcontrast(contrast) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t
parse_cmd_sgc_onoff(char *cmd, char *output, uint16_t len)
{
  char data[3];
  if ((sscanf_P(cmd, PSTR("%hhu"), &data[2]) != 1) || ((data[2] != 0) && (data[2] != 1)))       /* status only 1 or 0 */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x59;               /* "Control" Command */
  data[1] = 0x01;               /* "Display OnOff" Command */
  if (sgc_sendcommand(3, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* r, g, b */
parse_cmd_sgc_repbgc(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[4];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]);
  if (((ret == 3) && (rgb2sgc(&data[1], 3 - ret) != 0)) || ((ret != 3) && (ret != 2)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x42;               /* "Replace Background Colour" Command */
  if (sgc_sendcommand(3, data, OPT_LONG_ACK) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t
parse_cmd_sgc_cls(char *cmd, char *output, uint16_t len)
{
  char data[1];
  data[0] = 0x45;               /* "Clear Screen" Command */
  if (sgc_sendcommand(1, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* address, ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7 */
parse_cmd_sgc_adduc(char *cmd, char *output, uint16_t len)
{
  char data[11];
  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9]) != 9) || (data[1] > 0x1F)) /* address valid? */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x41;               /* "Add User Character" Command */
  if (sgc_sendcommand(10, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* pos_x, pos_y, radius, r, g, b */
parse_cmd_sgc_circle(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[7];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6]);
  if (((ret == 6) && (rgb2sgc(&data[4], 6 - ret) != 0)) || ((ret != 6) && (ret != 5)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x43;               /* "Draw Circle" Command */
  if (sgc_sendcommand(6, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* address, x, y, r, g, b */
parse_cmd_sgc_druc(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[7];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6]);
  if (((ret == 6) && (rgb2sgc(&data[4], 6 - ret) != 0)) ||
      ((ret != 6) && (ret != 5)) || (data[1] > 0x1F))
    return ECMD_ERR_PARSE_ERROR;        /* check colour + addr */
  data[0] = 0x44;               /* "Draw User Character" Command */
  if (sgc_sendcommand(6, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x1, y1, x2, y2, x3, y3, r, g, b */
parse_cmd_sgc_triangle(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[10];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
                 &data[7], &data[8], &data[9]);
  if (((ret == 9) && (rgb2sgc(&data[7], 9 - ret) != 0)) || ((ret != 9) && (ret != 8)) ||        /* check and convert colour */
      (data[1] <= data[3]) || (data[5] <= data[3]) ||   /* x1<=x2, x3<=x2 */
      (data[2] >= data[4]) || (data[2] >= data[6]))     /* y1>=y2, y1>=y3 */
    return ECMD_ERR_PARSE_ERROR;        /* counter-clockwise edges */
  data[0] = 0x47;               /* "Draw Triangle" Command */
  if (sgc_sendcommand(9, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /*r, g, b */
parse_cmd_sgc_setbgc(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[4];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]);
  if (((ret == 3) && (rgb2sgc(&data[1], 3 - ret) != 0)) || ((ret != 3) && (ret != 2)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x4B;               /* "Set Background Colour" Command */
  if (sgc_sendcommand(3, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x1, y1, x2, y2, r, g, b */
parse_cmd_sgc_line(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[8];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
                 &data[7]);
  if (((ret == 7) && (rgb2sgc(&data[5], 7 - ret) != 0)) || ((ret != 7) && (ret != 6)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x4C;               /* "Draw Line" Command */
  if (sgc_sendcommand(7, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x, y, r, g, b */
parse_cmd_sgc_pixel(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[6];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5]);
  if (((ret == 5) && (rgb2sgc(&data[3], 5 - ret) != 0)) || ((ret != 5) && (ret != 4)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x50;               /* "Draw Pixel" Command */
  if (sgc_sendcommand(5, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* source x, y, dest x, y, width, height */
parse_cmd_sgc_scrcp(char *cmd, char *output, uint16_t len)
{
  char data[7];
  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"),
               &data[1], &data[2], &data[3], &data[4], &data[5],
               &data[6]) != 6)
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x63;               /* "Screen Copy Paste" Command */
  if (sgc_sendcommand(7, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x1, y1, x2, y2, old r, g, b, new r, g, b */
parse_cmd_sgc_repcol(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[11];
  ret =
    sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
             &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
             &data[7], &data[8], &data[9], &data[10]);
  if (((ret == 10) && (rgb2sgc(&data[5], 10 - ret) != 0) && (rgb2sgc(&data[8], 10 - ret) != 0)) || ((ret != 10) && (ret != 8))) /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x6B;               /* "Replace Colour" Command */
  data[7] = data[8];            /* shift by 1 byte after conversion */
  data[8] = data[9];
  if (sgc_sendcommand(9, data, OPT_LONG_ACK) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* pensize (0 or 1) */
parse_cmd_sgc_pensize(char *cmd, char *output, uint16_t len)
{
  char data[2];
  if ((sscanf_P(cmd, PSTR("%hhu"), &data[1]) != 1) ||
      ((data[1] != 0) && (data[1] != 1)))
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x70;               /* "Pensize" Command */
  if (sgc_sendcommand(2, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x1, y1, x2, y2, r, g, b */
parse_cmd_sgc_rectangle(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[8];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
                 &data[7]);
  if (((ret == 7) && (rgb2sgc(&data[5], 7 - ret) != 0)) || ((ret != 7) && (ret != 6)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x72;               /* "Draw Rectangle" Command */
  if (sgc_sendcommand(7, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* font (0, 1, 2) */
parse_cmd_sgc_font(char *cmd, char *output, uint16_t len)
{
  char data[2];
  if ((sscanf_P(cmd, PSTR("%hhu"), &data[1]) != 1) || (data[1] > 2))    /* 0, 1 or 2 */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x46;               /* "Set Font" Command */
  if (sgc_sendcommand(2, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* opacity (0, 1) */
parse_cmd_sgc_opacity(char *cmd, char *output, uint16_t len)
{
  char data[2];
  if ((sscanf_P(cmd, PSTR("%hhu"), &data[1]) != 1) || (data[1] > 1))    /* only 0 or 1 valid */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x4F;               /* "Opacity" Command */
  if (sgc_sendcommand(2, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* x, y, width, height, sector add0, add1, add2 */
parse_cmd_sgc_sdicon(char *cmd, char *output, uint16_t len)
{
  char data[10];
  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
               &data[2], &data[3], &data[4], &data[5], &data[7], &data[8],
               &data[9]) != 7)
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x40;               /* extended command byte */
  data[1] = 0x49;               /* "Display Image Icon from SD" Command */
  data[6] = 0x10;               /* colour mode - only 65k mode possible */
  if (sgc_sendcommand(10, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* char, col, row, r, g, b */
parse_cmd_sgc_achar(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[7];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[4], &data[5], &data[6]);
  if (((ret == 6) && (rgb2sgc(&data[4], 6 - ret) != 0)) || ((ret != 6) && (ret != 5)) ||        /* check and convert colour */
      (data[1] < 0x20) || (data[1] > 0x7F) ||   /* valid font char range */
      (data[2] > 20) || (data[3] > 15)) /* max col and row values */
    return ECMD_ERR_PARSE_ERROR;
  /* could still lead to NACK depending on font size (see manual) */
  data[0] = 0x54;               /* "Draw ASCII Char" Command */
  if (sgc_sendcommand(6, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* char, x, y, width, height, r, g, b */
parse_cmd_sgc_gchar(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[9];
  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
                 &data[1], &data[2], &data[3], &data[7], &data[8], &data[4],
                 &data[5], &data[6]);
  if (((ret == 8) && (rgb2sgc(&data[4], 8 - ret) != 0)) || ((ret != 8) && (ret != 7)) ||        /* check and convert colour */
      (data[1] < 0x20) || (data[1] > 0x7F))     /* valid font char range */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x74;               /* "Draw Graphic Char" Command */
  data[6] = data[7];            /* shift to fit command format */
  data[7] = data[8];
  if (sgc_sendcommand(8, data, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;
  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

/*
  -- Ethersex META --
block([[SGC]] commands)
 ecmd_feature(sgc_set_powerstate, "sgc set powerstate", Set Power Status: 1 on 0:off)
 ecmd_feature(sgc_get_powerstate, "sgc get powerstate", Get Power Status)
   ecmd_ifdef(SGC_ECMD_SEND_SUPPORT)
 ecmd_feature(sgc_set_ip, "sgc set ip", Set response IP address (volatile))
  ecmd_endif()
  ecmd_ifdef(SGC_TIMEOUT_COUNTER_SUPPORT)
 ecmd_feature(sgc_set_timeout, "sgc set timeout", Set auto-off idle time (volatile))
  ecmd_endif()
 ecmd_feature(sgc_result, "sgc result", Get result from last command)
 ecmd_feature(sgc_contrast, "sgc contrast", Set contrast value 0..0x0F)
 ecmd_feature(sgc_onoff, "sgc onoff", Turn display on:1 or off:0)
 ecmd_feature(sgc_repbgc, "sgc repbgc", Replace background colour: red, green, blue 0..0x1F)
 ecmd_feature(sgc_cls, "sgc cls", Clear screen)
 ecmd_feature(sgc_adduc, "sgc adduc", Add user character: address 0..0x1F, 8 bytes character)
 ecmd_feature(sgc_circle, "sgc circle", Draw circle: x, y, radius, red, green, blue 0..0x1F)
 ecmd_feature(sgc_druc, "sgc druc", Draw user character: address 0..0x1F, x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_triangle, "sgc triangle", Draw triangle: x1, y1, x2, y2, x3, y3 counter-clockwise, red, green, blue 0..0x1F)
 ecmd_feature(sgc_setbgc, "sgc setbgc", Set background colour: red, green, blue 0..0x1F)
 ecmd_feature(sgc_line, "sgc line", Draw line: x1, y1, x2, y2, red, green, blue 0..0x1F)
 ecmd_feature(sgc_pixel, "sgc pixel", Draw pixel: x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_scrcp, "sgc scrcp", Copy-Paste screen area: x source, y source, x destination, y destination, width, height)
 ecmd_feature(sgc_repcol, "sgc repcol", Replace colour in area: x_start, y_start, x_end, y_end, old red, old green, old blue 0..0x1F, new red, new green, new blue 0..0x1F)
 ecmd_feature(sgc_pensize, "sgc pensize", Set pen size: 0:solid 1:lines)
 ecmd_feature(sgc_rectangle, "sgc rectangle", Draw rectangle: x_start, y_start, x_end, y_end, red, green, blue 0..0x1F)
 ecmd_feature(sgc_font, "sgc font", Set font type: 0..2)
 ecmd_feature(sgc_opacity, "sgc opacity", Set text opacity 0:transparent 1:opaque)
 ecmd_feature(sgc_sdicon, "sgc sdicon", Display icon from SD card:  x, y, width, height, sector addr0, addr1, addr2)
 ecmd_feature(sgc_achar, "sgc achar", Draw ASCII character:  char, column (0..20), row (0..15), red, green, blue 0..0x1F)
 ecmd_feature(sgc_gchar, "sgc gchar", Draw graphical ASCII character, char, x, y, width, height, red, green, blue 0..0x1F)
*/
