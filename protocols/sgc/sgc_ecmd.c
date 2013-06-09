/*
 * Copyright (c) 2013 by Nico Dziubek <hundertvolt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.

 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "protocols/sgc/sgc.h"
#include "protocols/sgc/sgc_ecmd.h"
#include "protocols/ecmd/ecmd-base.h"

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
    case WAKEUP:               /* 4 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("WAKEUP")));
    case NONE:                 /* 5 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("NONE")));
    default:                   /* 6 */
      return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
  }
}

int16_t                         /* set power mode (on / shutdown) */
parse_cmd_sgc_setpwr(char *cmd, char *output, uint16_t len)
{
  uint8_t power_soll;

  if ((sscanf_P(cmd, PSTR("%hhu"), &power_soll) != 1) ||
      ((power_soll != 0) && (power_soll != 1)))
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_setpowerstate(power_soll) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* get current power state */
parse_cmd_sgc_getpwr(char *cmd, char *output, uint16_t len)
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
int16_t                         /* change IP for sending ECMDs to (non-permanent) */
parse_cmd_sgc_setip(char *cmd, char *output, uint16_t len)
{
  if (sgc_setip(cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_ERR_PARSE_ERROR;
}
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
int16_t                         /* change value (in minuntes) for auto-shutdown (non-permanent) */
parse_cmd_sgc_settimeout(char *cmd, char *output, uint16_t len)
{
  uint8_t time;

  if ((sscanf_P(cmd, PSTR("%hhu"), &time) != 1) || (time == 0)) /* 1...255 minutes valid */
    return ECMD_ERR_PARSE_ERROR;

  sgc_settimeout(time);

  return ECMD_FINAL_OK;
}
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

int16_t                         /* standard auto-sleep mode: active (0, 1), serial / joystick (0 / 1) */
parse_cmd_sgc_setsleep(char *cmd, char *output, uint16_t len)
{
  char mode[2];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu"), &mode[0], &mode[1]) != 2) ||
      (mode[0] > 1) || (mode[1] > 1))
    return ECMD_ERR_PARSE_ERROR;

  mode[0] |= (mode[1] << 1);

  sgc_setshdnsleep(mode[0]);

  return ECMD_FINAL_OK;
}

int16_t                         /* turn display on and off */
parse_cmd_sgc_onoff(char *cmd, char *output, uint16_t len)
{
  char data[3];

  if ((sscanf_P(cmd, PSTR("%hhu"), &data[2]) != 1) || ((data[2] != 0) && (data[2] != 1)))       /* status only 1 or 0 */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x59;               /* "Control" Command */
  data[1] = 0x01;               /* "Display OnOff" Command */

  if (sgc_sendcommand(3, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* go to sleep mode, 0: wake on serial, 1: wake on joystick */
parse_cmd_sgc_sleep(char *cmd, char *output, uint16_t len)
{
  char mode;

  if ((sscanf_P(cmd, PSTR("%hhu"), &mode) != 1) || (mode > 1))  /* status only 1 or 0 */
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_sleep(mode, OPT_NORMAL) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* set contrast value */
parse_cmd_sgc_contrast(char *cmd, char *output, uint16_t len)
{
  char contrast;

  if ((sscanf_P(cmd, PSTR("%hhu"), &contrast) != 1) || (contrast > 0x0F))
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_setcontrast(contrast) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* set active colour (3-byte RGB 5:5:5 or 2-byte RGB 5:6:5) */
parse_cmd_sgc_colour(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char colour[3];

  ret =
    sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"), &colour[0], &colour[1],
             &colour[2]);

  if (((ret == 3) && (rgb2sgc(&colour[0], 3 - ret) != 0)) || ((ret != 3) && (ret != 2)))        /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;

  sgc_setcolour(colour);
  return ECMD_FINAL_OK;
}

int16_t                         /* set pensize (0:solid 1:lines) */
parse_cmd_sgc_pensize(char *cmd, char *output, uint16_t len)
{
  char pensize;

  if ((sscanf_P(cmd, PSTR("%hhu"), &pensize) != 1) || (pensize > 1))
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_setpensize(pensize) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* set font (0, 1, 2), proportional (0, 1) */
parse_cmd_sgc_font(char *cmd, char *output, uint16_t len)
{
  char font[2];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu"), &font[0], &font[1]) != 2) ||
      (font[0] > 2) || (font[1] > 1))
    return ECMD_ERR_PARSE_ERROR;
  font[0] = font[0] | (font[1] << 4);   /* OR with 0x1F: proportional */

  if (sgc_setfont(font[0]) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* set opacity (0, 1) */
parse_cmd_sgc_opacity(char *cmd, char *output, uint16_t len)
{
  char opacity;

  if ((sscanf_P(cmd, PSTR("%hhu"), &opacity) != 1) || (opacity > 1))    /* only 0 or 1 valid */
    return ECMD_ERR_PARSE_ERROR;

  if (sgc_setopacity(opacity) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* set background colour r, g, b */
parse_cmd_sgc_setbgc(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[4];

  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]);

  if (((ret == 3) && (rgb2sgc(&data[1], 3 - ret) != 0)) || ((ret != 3) && (ret != 2)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x4B;               /* "Set Background Colour" Command */

  if (sgc_sendcommand(3, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* replace background colour r, g, b */
parse_cmd_sgc_repbgc(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[4];

  ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]);

  if (((ret == 3) && (rgb2sgc(&data[1], 3 - ret) != 0)) || ((ret != 3) && (ret != 2)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
  data[0] = 0x42;               /* "Replace Background Colour" Command */

  if (sgc_sendcommand(3, FIVE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* replace colour x1, y1, x2, y2, old r, g, b */
parse_cmd_sgc_repcol(char *cmd, char *output, uint16_t len)
{
  uint8_t ret;
  char data[10];

  ret =
    sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &data[1],
             &data[2], &data[3], &data[4], &data[5], &data[6], &data[7]);

  if (((ret == 7) && (rgb2sgc(&data[5], 7 - ret) != 0)) || ((ret != 7) && (ret != 6)))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x6B;               /* "Replace Colour" Command */
  sgc_getcolour(&data[7]);      /* get colour setting */

  if (sgc_sendcommand(9, FIVE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* clear screen */
parse_cmd_sgc_cls(char *cmd, char *output, uint16_t len)
{
  char data[1];
  data[0] = 0x45;               /* "Clear Screen" Command */

  if (sgc_sendcommand(1, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* add user character, address, ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7 */
parse_cmd_sgc_adduc(char *cmd, char *output, uint16_t len)
{
  char data[11];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9]) != 9) || (data[1] > 0x1F)) /* address valid? */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x41;               /* "Add User Character" Command */

  if (sgc_sendcommand(10, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* screen copy-paste, source x, y, dest x, y, width, height */
parse_cmd_sgc_scrcp(char *cmd, char *output, uint16_t len)
{
  char data[7];

  if (sscanf_P
      (cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"), &data[1], &data[2],
       &data[3], &data[4], &data[5], &data[6]) != 6)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x63;               /* "Screen Copy Paste" Command */

  if (sgc_sendcommand(7, FIVE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw circle, pos_x, pos_y, radius */
parse_cmd_sgc_circle(char *cmd, char *output, uint16_t len)
{
  char data[6];

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]) !=
      3)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x43;               /* "Draw Circle" Command */
  sgc_getcolour(&data[4]);      /* get colour setting */

  if (sgc_sendcommand(6, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw user character, address, x, y */
parse_cmd_sgc_druc(char *cmd, char *output, uint16_t len)
{
  char data[6];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[1], &data[2], &data[3]) !=
       3) || (data[1] > 0x1F))
    return ECMD_ERR_PARSE_ERROR;        /* check colour + addr */

  data[0] = 0x44;               /* "Draw User Character" Command */
  sgc_getcolour(&data[4]);      /* get colour setting */

  if (sgc_sendcommand(6, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw triangle, x1, y1, x2, y2, x3, y3 */
parse_cmd_sgc_triangle(char *cmd, char *output, uint16_t len)
{
  char data[9];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"), &data[1], &data[2], &data[3], &data[4], &data[5], &data[6]) != 6) || (data[1] <= data[3]) || (data[5] <= data[3]) ||        /* x1<=x2, x3<=x2 */
      (data[2] >= data[4]) || (data[2] >= data[6]))     /* y1>=y2, y1>=y3 */
    return ECMD_ERR_PARSE_ERROR;        /* counter-clockwise edges */

  data[0] = 0x47;               /* "Draw Triangle" Command */
  sgc_getcolour(&data[7]);      /* get colour setting */

  if (sgc_sendcommand(9, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw line, x1, y1, x2, y2 */
parse_cmd_sgc_line(char *cmd, char *output, uint16_t len)
{
  char data[7];

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"),
               &data[1], &data[2], &data[3], &data[4]) != 4)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x4C;               /* "Draw Line" Command */
  sgc_getcolour(&data[5]);      /* get colour setting */

  if (sgc_sendcommand(7, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw line, x, y */
parse_cmd_sgc_pixel(char *cmd, char *output, uint16_t len)
{
  char data[5];

  if (sscanf_P(cmd, PSTR("%hhu %hhu"), &data[1], &data[2]) != 2)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x50;               /* "Draw Pixel" Command */
  sgc_getcolour(&data[3]);      /* get colour setting */

  if (sgc_sendcommand(5, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw rectangle, x1, y1, x2, y2 */
parse_cmd_sgc_rectangle(char *cmd, char *output, uint16_t len)
{
  char data[7];

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"),
               &data[1], &data[2], &data[3], &data[4]) != 4)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x72;               /* "Draw Rectangle" Command */
  sgc_getcolour(&data[5]);      /* get colour setting */

  if (sgc_sendcommand(7, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw "text" char, col, row, char */
parse_cmd_sgc_tchar(char *cmd, char *output, uint16_t len)
{
  char font[3], data[5];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &data[2], &data[3], &data[1]) != 3) || (data[1] < 0x20) || (data[1] > 0x7F) ||     /* valid font char range */
      (data[2] > 20) || ((data[2] > 15) && ((font[0] & 0x0F) != 0x00)) ||       /* valid col */
      (data[3] > 15) || ((data[3] > 9) && ((font[0] & 0x0F) == 0x02)))  /* valid row */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x54;               /* "Draw ASCII Char" Command */
  sgc_getcolour(&data[4]);      /* get colour setting */

  if (sgc_sendcommand(6, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw "grapical" char, x, y, width, height, char */
parse_cmd_sgc_gchar(char *cmd, char *output, uint16_t len)
{
  char data[8];

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu"), &data[2], &data[3], &data[6], &data[7], &data[1]) != 5) || (data[1] < 0x20) || (data[1] > 0x7F)) /* valid font char range */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x74;               /* "Draw Graphic Char" Command */
  sgc_getcolour(&data[4]);      /* get colour setting */

  if (sgc_sendcommand(8, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw "text" string, col, row, "string" */
parse_cmd_sgc_stt(char *cmd, char *output, uint16_t len)
{                               /* important: "sgc_stt" must be 7 characters long for calculation! */
  char data[7];
  uint16_t length;
  data[3] = sgc_getfont();

  if ((sscanf_P(cmd, PSTR("%hhu %hhu %n"), &data[1], &data[2], &length) != 2) || (data[1] > 20) || ((data[1] > 15) && ((data[3] & 0x0F) != 0x00)) ||    /* col */
      (data[2] > 15) || ((data[2] > 9) && ((data[3] & 0x0F) == 0x02)))  /* row */
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x73;               /* "Draw Graphics String" command */
  sgc_getcolour(&data[4]);      /* get colour setting */
  data[6] = (char) length;

  if (sgc_sendcommand(6, ONE_SEC, OPT_STRING, data, &cmd[data[6]]) == 0)
    return ECMD_FINAL_OK;       /* send whole command incl. string */

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* draw "grapical" string, x, y, width, height, "string" */
parse_cmd_sgc_stg(char *cmd, char *output, uint16_t len)
{                               /* important: "sgc_stg" must be 7 characters long for calculation! */
  char data[9];
  uint16_t length;
  data[3] = sgc_getfont();

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %n"),
               &data[1], &data[2], &data[6], &data[7], &length) != 4)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x53;               /* "Draw Graphics String" command */
  sgc_getcolour(&data[4]);      /* get colour setting */
  data[8] = (char) length;

  if (sgc_sendcommand(8, ONE_SEC, OPT_STRING, data, &cmd[data[8]]) == 0)
    return ECMD_FINAL_OK;       /* send whole command incl. string */

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* display SD icon, x, y, width, height, sector add0, add1, add2 */
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

  if (sgc_sendcommand(10, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* display SD video, x, y, width, height, delay, frames (msb:lsb), addr (hi:mid:lo) */
parse_cmd_sgc_video(char *cmd, char *output, uint16_t len)
{
  char data[13];
  uint32_t runtime;
  uint16_t temp = 0;

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"),
               &data[2], &data[3], &data[4], &data[5], &data[7],
               &data[8], &data[9], &data[10], &data[11], &data[12]) != 10)
    return ECMD_ERR_PARSE_ERROR;

  temp += data[8];
  temp = temp << 8;
  temp += data[9];              /* 16bit number of frames */

  runtime = temp * (2 + data[7]);       /* time in ms: 2ms per frame + inter-frame delay */
  runtime = (((3 * runtime) + ((5 * runtime) >> 8)) >> 6);      /* approx. division by 20ms */
  runtime += 1500;              /* additional 30 sec just in case */

  if (runtime > 0x0000FFFF)
    return ECMD_FINAL(snprintf_P(output, len, PSTR("TOO_LONG")));

  temp = (uint16_t) runtime;
  data[0] = 0x40;               /* "Extended Command" Command */
  data[1] = 0x56;               /* "Video" Command */
  data[6] = 0x10;               /* "65k Colour mode" Command */

  if (sgc_sendcommand(13, temp, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* display SD object, Umsb, Ulsb, Lmsb, Llsb */
parse_cmd_sgc_object(char *cmd, char *output, uint16_t len)
{
  char data[6];

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"),
               &data[2], &data[3], &data[4], &data[5]) != 4)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x40;               /* "Extended Command" Command */
  data[1] = 0x4F;               /* "Display Object" Command */

  if (sgc_sendcommand(6, ONE_SEC, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t                         /* execute SD script, Umsb, Ulsb, Lmsb, Llsb */
parse_cmd_sgc_script(char *cmd, char *output, uint16_t len)
{
  char data[6];

  if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu"),
               &data[2], &data[3], &data[4], &data[5]) != 4)
    return ECMD_ERR_PARSE_ERROR;

  data[0] = 0x40;               /* "Extended Command" Command */
  data[1] = 0x50;               /* "Display Object" Command */

  if (sgc_sendcommand(6, INFINITE, OPT_NORMAL, data, cmd) == 0)
    return ECMD_FINAL_OK;

  return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

/*
 -- Ethersex META --
block([[SGC]] commands)
 ecmd_feature(sgc_result, "sgc result", Get result from last command)
 ecmd_feature(sgc_setpwr, "sgc setpwr", Set Power Status: 1 on 0:off)
 ecmd_feature(sgc_getpwr, "sgc getpwr", Get Power Status)
 ecmd_ifdef(SGC_ECMD_SEND_SUPPORT)
 ecmd_feature(sgc_setip, "sgc setip", Set response IP address (volatile))
 ecmd_endif()
 ecmd_ifdef(SGC_TIMEOUT_COUNTER_SUPPORT)
 ecmd_feature(sgc_settimeout, "sgc settimeout", Set auto-off idle time (volatile))
 ecmd_endif()
 ecmd_feature(sgc_setsleep, "sgc setsleep", Set Auto Shutdown Sleep on (1) / off (0), Set wakeup mode (0: Serial, 1: Joystick))
 ecmd_feature(sgc_onoff, "sgc onoff", Turn display on:1 or off:0)
 ecmd_feature(sgc_sleep, "sgc sleep", Sleep mode, 0: turn off SD, 1: wake on serial, 2: wake on joystick)
 ecmd_feature(sgc_contrast, "sgc contrast", Set contrast value 0..0x0F)
 ecmd_feature(sgc_colour, "sgc colour", Set pen / font colour: red, green, blue 0..0x1F)
 ecmd_feature(sgc_pensize, "sgc pensize", Set pen size: 0:solid 1:lines)
 ecmd_feature(sgc_font, "sgc font", Set font type: 0..2)
 ecmd_feature(sgc_opacity, "sgc opacity", Set text opacity 0:transparent 1:opaque)
 ecmd_feature(sgc_setbgc, "sgc setbgc", Set background colour: red, green, blue 0..0x1F)
 ecmd_feature(sgc_repbgc, "sgc repbgc", Replace background colour: red, green, blue 0..0x1F)
 ecmd_feature(sgc_repcol, "sgc repcol", Replace colour in area: x_start, y_start, x_end, y_end, old red, old green, old blue 0..0x1F)
 ecmd_feature(sgc_cls, "sgc cls", Clear screen)
 ecmd_feature(sgc_adduc, "sgc adduc", Add user character: address 0..0x1F, 8 bytes character)
 ecmd_feature(sgc_scrcp, "sgc scrcp", Copy-Paste screen area: x source, y source, x destination, y destination, width, height)
 ecmd_feature(sgc_circle, "sgc circle", Draw circle: x, y, radius, red, green, blue 0..0x1F)
 ecmd_feature(sgc_druc, "sgc druc", Draw user character: address 0..0x1F, x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_triangle, "sgc triangle", Draw triangle: x1, y1, x2, y2, x3, y3 counter-clockwise, red, green, blue 0..0x1F)
 ecmd_feature(sgc_line, "sgc line", Draw line: x1, y1, x2, y2, red, green, blue 0..0x1F)
 ecmd_feature(sgc_pixel, "sgc pixel", Draw pixel: x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_rectangle, "sgc rectangle", Draw rectangle: x_start, y_start, x_end, y_end, red, green, blue 0..0x1F)
 ecmd_feature(sgc_tchar, "sgc tchar", Draw text ASCII character: char, column (0..20), row (0..15), red, green, blue 0..0x1F)
 ecmd_feature(sgc_gchar, "sgc gchar", Draw graphical ASCII character, char, x, y, width, height, red, green, blue 0..0x1F)
 ecmd_feature(sgc_stg, "sgc stg", Draw graphical ASCII string, x, y, width, height, red, green, blue 0..0x1F, string)
 ecmd_feature(sgc_stt, "sgc stt", Draw graphical ASCII string, col, row, red, green, blue 0..0x1F, string)
 ecmd_feature(sgc_sdicon, "sgc sdicon", Display icon from SD card: x, y, width, height, SD sector addr0, addr1, addr2)
 ecmd_feature(sgc_video, "sgc video", Display video / animation from SD card, x, y, width, height, frame delay [ms], num frames (16bit), SD sector addr0, addr1, addr2)
 ecmd_feature(sgc_object, "sgc object", Display object from SD card, SD address: Umsb, Ulsb, Lmsb, Llsb )
 ecmd_feature(sgc_script, "sgc script", Run 4DSL script from SD card, SD address: Umsb, Ulsb, Lmsb, Llsb )
*/
