/*
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
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

#define STELLA_FLAG_SORT 1
#define STELLA_FLAG_ACK 2
#define STELLA_PROTOCOL_VERSION 3

enum stella_colors
{
  STELLA_COLOR_0,
  STELLA_COLOR_1,
  STELLA_COLOR_2,
  STELLA_COLOR_3,
  STELLA_COLOR_4,
  STELLA_COLOR_5,
  STELLA_COLOR_6,
  STELLA_COLOR_7
};

enum stella_commands
{
  STELLA_SET_COLOR_0=0,
  STELLA_SET_COLOR_1,
  STELLA_SET_COLOR_2,
  STELLA_SET_COLOR_3,
  STELLA_SET_COLOR_4,
  STELLA_SET_COLOR_5,
  STELLA_SET_COLOR_6,
  STELLA_SET_COLOR_7,
  STELLA_FADE_COLOR_0,
  STELLA_FADE_COLOR_1,
  STELLA_FADE_COLOR_2,
  STELLA_FADE_COLOR_3,
  STELLA_FADE_COLOR_4,
  STELLA_FADE_COLOR_5,
  STELLA_FADE_COLOR_6,
  STELLA_FADE_COLOR_7,
  STELLA_FLASH_COLOR_0,
  STELLA_FLASH_COLOR_1,
  STELLA_FLASH_COLOR_2,
  STELLA_FLASH_COLOR_3,
  STELLA_FLASH_COLOR_4,
  STELLA_FLASH_COLOR_5,
  STELLA_FLASH_COLOR_6,
  STELLA_FLASH_COLOR_7,
  STELLA_SELECT_FADE_FUNC,
  STELLA_FADE_STEP,
  STELLA_ACK_RESPONSE,
  STELLA_UNICAST_RESPONSE,
  STELLA_BROADCAST_RESPONSE,
  STELLA_SAVE_TO_EEPROM,
  STELLA_LOAD_FROM_EEPROM,
  STELLA_COUNT_CRONJOBS,
  STELLA_GET_CRONJOBS,
  STELLA_GET_CRONJOB,
  STELLA_RM_CRONJOB,
  STELLA_ADD_CRONJOB
};

enum
{
	STELLA_FADE_NORMAL,
	STELLA_FADE_FLASHY,
	FADE_FUNC_LEN
};

extern uint8_t stella_color[];
extern uint8_t stella_fade[];

extern volatile uint8_t stella_fade_counter;
extern uint8_t stella_fade_step;
extern uint8_t stella_net_ack;

/* stella.c */
void cron_stella_callback(void* data);
void stella_init(void);
void stella_process(void);
void stella_newdata(unsigned char *buf, uint8_t len);
/* stella_pwm.c */
void stella_sort(uint8_t color[]);

