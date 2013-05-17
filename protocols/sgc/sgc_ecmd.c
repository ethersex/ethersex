/*
 *
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

int16_t parse_cmd_sgc_set_powerstate(char *cmd, char *output, uint16_t len)
{ 
 uint8_t power_soll;
 if ((sscanf_P(cmd, PSTR("%hhu"), &power_soll) != 1) || ((power_soll != 0) && (power_soll != 1)))
  return ECMD_ERR_PARSE_ERROR;
 if (sgc_setpowerstate(power_soll) == 0)
   return ECMD_FINAL_OK;
 return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_get_powerstate(char *cmd, char *output, uint16_t len)
{ 
 uint8_t pwrstate;
 pwrstate = sgc_getpowerstate();
 if (pwrstate == SHUTDOWN)
  return ECMD_FINAL(snprintf_P(output, len, PSTR("SHUTDOWN")));
  if (pwrstate == POWERUP)
  return ECMD_FINAL(snprintf_P(output, len, PSTR("POWERUP")));
 return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

#ifdef SGC_TIMEOUT_COUNTER
int16_t parse_cmd_sgc_reset_timeout(char *cmd, char *output, uint16_t len)
{
sgc_reset_timeout(); 
return ECMD_FINAL_OK;
}
#endif /* SGC_TIMEOUT_COUNTER */

int16_t parse_cmd_sgc_get_command_result(char *cmd, char *output, uint16_t len)
{ 
 uint8_t cmdres;
 cmdres = sgc_getcommandresult();
 switch (cmdres)
  {
  case FROM_RESET: /* 0 */
  	return ECMD_FINAL(snprintf_P(output, len, PSTR("RESET")));
  case ACK: /* 1 */
   return ECMD_FINAL(snprintf_P(output, len, PSTR("ACK")));
  case NACK: /* 2 */
   return ECMD_FINAL(snprintf_P(output, len, PSTR("NACK")));
  case SENDING: /* 3 */
   return ECMD_FINAL(snprintf_P(output, len, PSTR("SENDING")));
  case NONE: /* 4 */
   return ECMD_FINAL(snprintf_P(output, len, PSTR("NONE")));
  default: /* 5 */
    return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
  }
}

int16_t parse_cmd_sgc_set_contrast(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret;
 uint8_t contrast;

 if ((sscanf_P(cmd, PSTR("%hhu"), &contrast) != 1) || (contrast > 0x0F))
  return ECMD_ERR_PARSE_ERROR;

if (sgc_setcontrast(contrast) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));  
}

int16_t parse_cmd_sgc_disp_onoff(char *cmd, char *output, uint16_t len)
{ 
 uint8_t status, data[SGC_BUFFER_LENGTH];

 if ((sscanf_P(cmd, PSTR("%hhu"), &status) != 1) || ((status != 0) && (status != 1))) /* nur 0 oder 1 zulässig */
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x59;  /* "Control" Command */
 data[1] = 0x01;  /* "Display OnOff" Command */
 data[2] = status;  /* Status Command */
ECMD_ERR_WRITE_ERROR;
if (sgc_sendcommand(3, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_replace_background_colour(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, red, green, blue, data[SGC_BUFFER_LENGTH], sgc_colour[2];
 
 ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &red, &green, &blue);
  if (ret == 2)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 3) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 3))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;

 data[0] = 0x42;      /* "Replace Background Colour" Command */
 data[1] = sgc_colour[0];
 data[2] = sgc_colour[1];

if (sgc_sendcommand(3, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_clear_screen(char *cmd, char *output, uint16_t len)
{ 
 uint8_t data[SGC_BUFFER_LENGTH];
 
 data[0] = 0x45;      /* "Clear Screen" Command */

if (sgc_sendcommand(1, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_add_user_character(char *cmd, char *output, uint16_t len)
{ 
 uint8_t address, ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, data[SGC_BUFFER_LENGTH];

 if ((sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &address, &ch0, &ch1, &ch2, &ch3, &ch4, &ch5, &ch6, &ch7) != 9) || (address > 0x1F))  /* 32 addresses available */
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x41;      /* "Add User Character" Command */
 data[1] = address;
 data[2] = ch0;
 data[3] = ch1;
 data[4] = ch2;
 data[5] = ch3;
 data[6] = ch4;
 data[7] = ch5;
 data[8] = ch6;
 data[9] = ch7;

if (sgc_sendcommand(10, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_circle(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, center_x, center_y, radius, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];;

 ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"), &center_x, &center_y, &radius, &red, &green, &blue);
  if (ret == 5)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 6) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 6))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x43;  /* "Draw Circle" Command */
 data[1] = center_x;
 data[2] = center_y;
 data[3] = radius;
 data[4] = sgc_colour[0];
 data[5] = sgc_colour[1];

if (sgc_sendcommand(6, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_user_character(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, address, pos_x, pos_y, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];

 ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"), &address, &pos_x, &pos_y, &red, &green, &blue);
  if (ret == 5)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 6) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 6) || (address > 0x1F))  /* check and convert colour; 32 addresses available */
    return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x44;  /* "Draw User Character" Command */
 data[1] = address;
 data[2] = pos_x;
 data[3] = pos_y;
 data[4] = sgc_colour[0];
 data[5] = sgc_colour[1];

if (sgc_sendcommand(6, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_triangle(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, pos_x1, pos_y1, pos_x2, pos_y2, pos_x3, pos_y3, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];
 
 ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &pos_x1, &pos_y1, &pos_x2, &pos_y2, &pos_x3, &pos_y3, &red, &green, &blue);
  if (ret == 8)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 9) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 9) || ((pos_x1 <= pos_x2) || (pos_x3 <= pos_x2) || (pos_y1 >= pos_y2) || (pos_y3 <= pos_y1)))  
    return ECMD_ERR_PARSE_ERROR; /* check and convert colour; triangle must be defined counter-clockwise (see SGC manual) */
  
 data[0] = 0x47;  /* "Draw Triangle" Command */
 data[1] = pos_x1;
 data[2] = pos_y1;
 data[3] = pos_x2;
 data[4] = pos_y2;
 data[5] = pos_x3;
 data[6] = pos_y3;
 data[7] = sgc_colour[0];
 data[8] = sgc_colour[1];

if (sgc_sendcommand(9, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_set_background_colour(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];
 
 ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &red, &green, &blue);
  if (ret == 2)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 3) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 3))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x4B;      /* "Set Background Colour" Command */
 data[1] = sgc_colour[0];
 data[2] = sgc_colour[1];

if (sgc_sendcommand(3, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_line(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, pos_x1, pos_y1, pos_x2, pos_y2, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];

ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &pos_x1, &pos_y1, &pos_x2, &pos_y2, &red, &green, &blue);
  if (ret == 6)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 7) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 7))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x4C;  /* "Draw Line" Command */
 data[1] = pos_x1;
 data[2] = pos_y1;
 data[3] = pos_x2;
 data[4] = pos_y2;
 data[5] = sgc_colour[0];
 data[6] = sgc_colour[1];

if (sgc_sendcommand(7, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_pixel(char *cmd, char *output, uint16_t len)
{ 
uint8_t ret, pos_x, pos_y, red, green, blue, sgc_colour[2], data[SGC_BUFFER_LENGTH];

ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu"), &pos_x, &pos_y, &red, &green, &blue);
  if (ret == 4)
    {
    sgc_colour[0] = red;
    sgc_colour[1] = green;
    }
  else if (((ret == 5) && (rgb2sgc(red, green, blue, sgc_colour) != 0)) || (ret != 5))  /* check and convert colour */
    return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x50;  /* "Draw Pixel" Command */
 data[1] = pos_x;
 data[2] = pos_y;
 data[3] = sgc_colour[0];
 data[4] = sgc_colour[1];

if (sgc_sendcommand(5, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_copy_paste(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret;
 uint8_t data[SGC_BUFFER_LENGTH];
 uint8_t x_source, y_source, x_dest, y_dest, width, height;

 if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu"), &x_source, &y_source, &x_dest, &y_dest, &width, &height) != 6)
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x63;  /* "Screen Copy Paste" Command */
 data[1] = x_source;
 data[2] = y_source;
 data[3] = x_dest;
 data[4] = y_dest;
 data[5] = width;
 data[6] = height;

if (sgc_sendcommand(7, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_polygon(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret, parse_res, datacnt;
 uint8_t data[SGC_BUFFER_LENGTH];
 uint8_t edges, pos_x1, pos_y1, pos_x2, pos_y2, pos_x3, pos_y3, pos_x4, pos_y4, pos_x5, pos_y5, pos_x6, pos_y6, pos_x7, pos_y7, red, green, blue;
 uint8_t sgc_colour[2];

  parse_res = sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &edges, &pos_x1, &pos_y1, &pos_x2, &pos_y2, &pos_x3, &pos_y3, &pos_x4, &pos_y4, &pos_x5, &pos_y5, &pos_x6, &pos_y6, &pos_x7, &pos_y7, &red, &green, &blue);
 if ((edges < 3) || (edges > 7))
  return ECMD_ERR_PARSE_ERROR;
 if (parse_res != (4 + 2 * edges))
  return ECMD_ERR_PARSE_ERROR;
 if (rgb2sgc(red, green, blue, sgc_colour) != 0) /* check and convert colour */
  return ECMD_ERR_PARSE_ERROR;
 
 datacnt = 7;
 data[0] = 0x67;  /* "Draw Polygon" Command */
 data[1] = pos_x1;
 data[2] = pos_y1;
 data[3] = pos_x2;
 data[4] = pos_y2;
 data[5] = pos_x3;
 data[6] = pos_y3;
 if (edges > 3)
  {
  datacnt = 9;
  data[7] = pos_x4;
  data[8] = pos_y4; 
  }
 if (edges > 4)
  {
  datacnt = 11;
  data[9] = pos_x5;
  data[10] = pos_y5; 
  }
 if (edges > 5)
  {
  datacnt = 13;
  data[11] = pos_x6;
  data[12] = pos_y6; 
  }
 if (edges > 6)
  {
  datacnt = 15;
  data[13] = pos_x7;
  data[14] = pos_y7; 
  }
 data[datacnt] = sgc_colour[0];
 data[datacnt + 1] = sgc_colour[1];

if (sgc_sendcommand(datacnt + 2, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_replace_colour(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret;
 uint8_t data[SGC_BUFFER_LENGTH];
 uint8_t pos_x1, pos_y1, pos_x2, pos_y2, red_old, green_old, blue_old, red_new, green_new, blue_new;
 uint8_t sgc_colour_old[2], sgc_colour_new[2];

 if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &pos_x1, &pos_y1, &pos_x2, &pos_y2, &red_old, &green_old, &blue_old, &red_new, &green_new, &blue_new) != 10)
  return ECMD_ERR_PARSE_ERROR;
  
 if (rgb2sgc(red_old, green_old, blue_old, sgc_colour_old) != 0) /* check and convert colour */
  return ECMD_ERR_PARSE_ERROR;

 if (rgb2sgc(red_new, green_new, blue_new, sgc_colour_new) != 0) /* check and convert colour */
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x6B;  /* "Replace Colour" Command */
 data[1] = pos_x1;
 data[2] = pos_y1;
 data[3] = pos_x2;
 data[4] = pos_y2;
 data[5] = sgc_colour_old[0];
 data[6] = sgc_colour_old[1];
 data[7] = sgc_colour_new[0];
 data[8] = sgc_colour_new[1];

if (sgc_sendcommand(9, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_set_pen_size(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret;
 uint8_t data[SGC_BUFFER_LENGTH];
 uint8_t pensize;

 if (sscanf_P(cmd, PSTR("%hhu"), &pensize) != 1)
  return ECMD_ERR_PARSE_ERROR;
  
 if ((pensize != 0) && (pensize != 1))
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x70;  /* "Pensize" Command */
 data[1] = pensize;

if (sgc_sendcommand(2, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

int16_t parse_cmd_sgc_draw_rectangle(char *cmd, char *output, uint16_t len)
{ 
 uint8_t ret;
 uint8_t data[SGC_BUFFER_LENGTH];
 uint8_t pos_x1, pos_y1, pos_x2, pos_y2, red, green, blue;
 uint8_t sgc_colour[2];

 if (sscanf_P(cmd, PSTR("%hhu %hhu %hhu %hhu %hhu %hhu %hhu"), &pos_x1, &pos_y1, &pos_x2, &pos_y2, &red, &green, &blue) != 7)
  return ECMD_ERR_PARSE_ERROR;
  
 if (rgb2sgc(red, green, blue, sgc_colour) != 0) /* check and convert colour */
  return ECMD_ERR_PARSE_ERROR;
 
 data[0] = 0x72;  /* "Draw Rectangle" Command */
 data[1] = pos_x1;
 data[2] = pos_y1;
 data[3] = pos_x2;
 data[4] = pos_y2;
 data[5] = sgc_colour[0];
 data[6] = sgc_colour[1];

if (sgc_sendcommand(7, data, 0) == 0)
	return ECMD_FINAL_OK;
return ECMD_FINAL(snprintf_P(output, len, PSTR("BUSY")));
}

/*
  -- Ethersex META --
block([[SGC]] commands)
 ecmd_feature(sgc_set_powerstate, "sgc set powerstate", Set Power Status, 0:Reset 1:Power Down 2:Power Up)
 ecmd_feature(sgc_get_powerstate, "sgc get powerstate", Get Power Status)
 ecmd_ifdef(SGC_TIMEOUT_COUNTER)
 ecmd_feature(sgc_reset_timeout, "sgc reset timeout", Reset auto-off timeout)
 ecmd_endif()
 ecmd_feature(sgc_get_command_result, "sgc get command result", Get result from last command)
 ecmd_feature(sgc_set_contrast, "sgc set contrast", set contrast value 0..0x0F)
 ecmd_feature(sgc_disp_onoff, "sgc disp onoff", Turn display on:1 or off:0)
 ecmd_feature(sgc_replace_background_colour, "sgc replace background colour", red, green, blue 0..0x1F)
 ecmd_feature(sgc_clear_screen, "sgc clear screen", Clear screen)
 ecmd_feature(sgc_add_user_character, "sgc add user character", address 0..0x1F, 8 bytes character)
 ecmd_feature(sgc_draw_circle, "sgc draw circle", x, y, radius, red, green, blue 0..0x1F)
 ecmd_feature(sgc_draw_user_character, "sgc draw user character", address 0..0x1F, x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_draw_triangle, "sgc draw triangle", x1, y1, x2, y2, x3, y3 counter-clockwise, red, green, blue 0..0x1F)
 ecmd_feature(sgc_set_background_colour, "sgc set background colour", red, green, blue 0..0x1F)
 ecmd_feature(sgc_draw_line, "sgc draw line", x1, y1, x2, y2, red, green, blue 0..0x1F)
 ecmd_feature(sgc_draw_pixel, "sgc draw pixel", x, y, red, green, blue 0..0x1F)
 ecmd_feature(sgc_copy_paste, "sgc copy paste", x source, y source, x destination, y destination, width, height)
 ecmd_feature(sgc_draw_polygon, "sgc draw polygon", number of edges:3..7, x_edge1..n, y_edge1..n, red, green, blue 0..0x1F)
 ecmd_feature(sgc_replace_colour, "sgc replace colour", x_start, y_start, x_end, y_end, old red, old green, old blue 0..0x1F, new red, new green, new blue 0..0x1F)
 ecmd_feature(sgc_set_pen_size, "sgc set pen size", 0:solid 1:lines)
 ecmd_feature(sgc_draw_rectangle, "sgc draw rectangle", x_start, y_start, x_end, y_end, red, green, blue 0..0x1F)
*/
