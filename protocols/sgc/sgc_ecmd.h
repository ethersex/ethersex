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
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SGC_ECMD_H_
#define __SGC_ECMD_H_

int16_t parse_cmd_sgc_result(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_setpwr(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_getpwr(char *cmd, char *output, uint16_t len);

#ifdef SGC_ECMD_SEND_SUPPORT
int16_t parse_cmd_sgc_setip(char *cmd, char *output, uint16_t len);
#endif /* SGC_ECMD_SEND_SUPPORT */

#ifdef SGC_TIMEOUT_COUNTER_SUPPORT
int16_t parse_cmd_sgc_settimeout(char *cmd, char *output, uint16_t len);
#endif /* SGC_TIMEOUT_COUNTER_SUPPORT */

int16_t parse_cmd_sgc_setsleep(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_onoff(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_sleep(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_contrast(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_colour(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_pensize(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_font(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_opacity(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_setbgc(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_repbgc(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_repcol(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_cls(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_adduc(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_scrcp(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_circle(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_druc(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_triangle(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_line(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_pixel(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_rectangle(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_tchar(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_gchar(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_stt(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_stg(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_sdicon(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_video(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_object(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_sgc_script(char *cmd, char *output, uint16_t len);

#endif /* __SGC_ECMD_H_ */
