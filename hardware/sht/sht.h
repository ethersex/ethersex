/*
*
* Copyright (c) 2010 Gerd v. Egidy <gerd@egidy.de>
* Copyright (c) 2013 Erik Kunze <ethersex@erik-kunze.de>
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

#ifndef SHT_H
#define SHT_H

#ifdef SHT_SUPPORT

#define SHT_OK 0
#define SHT_ERR_TIMEOUT -1
#define SHT_ERR_PROTOCOL -2
#define SHT_ERR_CRC -3

#define SHT_CMD_TEMP    0x03
#define SHT_CMD_HUMID   0x05
#define SHT_CMD_WR_STAT 0x06
#define SHT_CMD_RD_STAT 0x07
#define SHT_CMD_RESET   0x1E

#define SHT_VOLTAGE_COMPENSATION_D1_5V -40.1
#define SHT_VOLTAGE_COMPENSATION_D1_4V -39.8
#define SHT_VOLTAGE_COMPENSATION_D1_3_5V -39.7
#define SHT_VOLTAGE_COMPENSATION_D1_3_3V -39.7
#define SHT_VOLTAGE_COMPENSATION_D1_3V -39.6
#define SHT_VOLTAGE_COMPENSATION_D1_2_5V -39.4

#if SHT_FREQUENCY > 1000000
#error "SHT frequency too high"
#endif

// 1/4 freq because we wait 4 times during one SCK cycle
#define SHT_DELAY_US (1000000 / (SHT_FREQUENCY / 3))

int8_t sht_get(uint16_t * raw_temp, uint16_t * raw_humid);
int16_t sht_convert_temp(uint16_t const *raw_temp);
uint16_t sht_convert_humid(uint16_t const *raw_temp,
                           uint16_t const *raw_humid);

#endif /* SHT_SUPPORT */

#endif /* SHT_H */
