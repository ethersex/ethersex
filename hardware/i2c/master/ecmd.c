/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <string.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_master.h"
#include "hardware/i2c/master/i2c_generic.h"
#include "hardware/i2c/master/i2c_lm75.h"
#include "hardware/i2c/master/i2c_ds1631.h"
#include "hardware/i2c/master/i2c_tsl2550.h"
#include "hardware/i2c/master/i2c_24CXX.h"
#include "hardware/i2c/master/i2c_pca9531.h"
#include "hardware/i2c/master/i2c_pcf8574x.h"
#include "hardware/i2c/master/i2c_max7311.h"

#include "protocols/ecmd/ecmd-base.h"

#ifdef I2C_DETECT_SUPPORT

int16_t parse_cmd_i2c_detect(char *cmd, char *output, uint16_t len)
{
	/* First call, we initialize our magic bytes*/
	if (cmd[0] != 0x23)
	{
		cmd[0] = 0x23;
		cmd[1] = 0;
	}
	uint8_t next_address = i2c_master_detect(cmd[1], 127);
	cmd[1] = next_address + 1;

	if (next_address > 127) /* End of scaning */
		return ECMD_FINAL_OK;
	else
		return ECMD_AGAIN(snprintf_P(output, len, PSTR("detected at: 0x%x (%d)"), next_address, next_address));
}

#endif  /* I2C_DETECT_SUPPORT */

#ifdef I2C_GENERIC_SUPPORT

int16_t parse_cmd_i2c_read_byte(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	sscanf_P(cmd, PSTR("%u"), &adr);
	if (adr < 7 || adr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint8_t val = i2c_read_byte(adr);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c rbb %d 0x%X"), adr, val));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), val));
#endif
}

int16_t parse_cmd_i2c_read_byte_data(char *cmd, char *output, uint16_t len)
{
	uint8_t cadr;
	uint8_t dadr;
	sscanf_P(cmd, PSTR("%u %u"), &cadr, &dadr);
	if (cadr < 7 || cadr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint8_t val = i2c_read_byte_data(cadr, dadr);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c rbd %d %d 0x%X"), cadr, dadr, val));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), val));
#endif
}

int16_t parse_cmd_i2c_read_word_data(char *cmd, char *output, uint16_t len)
{
	uint8_t cadr;
	uint8_t dadr;
	sscanf_P(cmd, PSTR("%u %u"), &cadr, &dadr);
	if (cadr < 7 || cadr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t val = i2c_read_word_data(cadr, dadr);
#ifdef ECMD_MIRROR_REQUEST
		return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c rwd %d %d 0x%X"), cadr, dadr, val));
#else
		return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), val));
#endif
}

int16_t parse_cmd_i2c_write_byte(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	uint8_t data;
	sscanf_P(cmd, PSTR("%u %x"), &adr, &data);
	if (adr < 7 || adr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t ret = i2c_write_byte(adr, data);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c wbb %d 0x%X"), adr, ret));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), ret));
#endif
}

int16_t parse_cmd_i2c_write_byte_data(char *cmd, char *output, uint16_t len)
{
	uint8_t cadr;
	uint8_t dadr;
	uint8_t data;
	sscanf_P(cmd, PSTR("%u %u %x"), &cadr, &dadr, &data);
	if (cadr < 7 || cadr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t ret = i2c_write_byte_data(cadr, dadr, data);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c wbd %d %d 0x%X"), cadr, dadr, ret));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), ret));
#endif
}

int16_t parse_cmd_i2c_write_word_data(char *cmd, char *output, uint16_t len)
{
	uint8_t cadr;
	uint8_t dadr;
	uint16_t data;
	sscanf_P(cmd, PSTR("%u %u %x"), &cadr, &dadr, &data);
	if (cadr < 7 || cadr > 127)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t ret = i2c_write_word_data(cadr, dadr, data);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("i2c wwd %d %d 0x%X"), cadr, dadr, ret));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), ret));
#endif
}

#endif /* I2C_GENERIC_SUPPORT */

#ifdef I2C_LM75_SUPPORT

int16_t
parse_cmd_i2c_lm75(char *cmd, char *output, uint16_t len)
{
	while(*cmd == ' ') cmd++;
	if (*cmd < '0' || *cmd > '7') return ECMD_ERR_PARSE_ERROR;
	int16_t temp = i2c_lm75_read_temp(I2C_SLA_LM75 + (cmd[0] - '0'));
	if (temp == 0xffff)
	return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("lm75 %d %d.%d"), (cmd[0] - '0'), temp / 10, temp % 10));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d.%d"), temp / 10, temp % 10));
#endif
}

#endif  /* I2C_LM75_SUPPORT */

#ifdef I2C_DS1631_SUPPORT

int16_t parse_cmd_i2c_ds1631_start(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	sscanf_P(cmd, PSTR("%u"), &adr);
	if (adr > 7)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t temp = i2c_ds1631_start_stop(I2C_SLA_DS1631 + adr,1);
	if (temp == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ds1631 start %d ok"),adr));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
#endif
}

int16_t parse_cmd_i2c_ds1631_stop(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	sscanf_P(cmd, PSTR("%u"), &adr);
	if (adr > 7)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t temp = i2c_ds1631_start_stop(I2C_SLA_DS1631 + adr,0);
	if (temp == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ds1631 stop %d ok"),adr));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
#endif
}

int16_t parse_cmd_i2c_ds1631_read_temp(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	int16_t temp;
	int16_t stemp;
	sscanf_P(cmd, PSTR("%u"), &adr);
	if (adr > 7)
		return ECMD_ERR_PARSE_ERROR;
	uint16_t ret = i2c_ds1631_read_temp(I2C_SLA_DS1631 + adr, &temp, &stemp);
	if (ret == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ds1631 temp %d %d.%d"), adr, temp, stemp));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d.%d"), temp, stemp));
#endif
}

#endif  /* I2C_DS1631_SUPPORT */

#ifdef I2C_TSL2550_SUPPORT

int16_t parse_cmd_i2c_tsl2550_pwr_up(char *cmd, char *output, uint16_t len)
{
	uint16_t temp = i2c_tsl2550_pwr(1);
	if (temp == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("tsl2550 up ok")));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
#endif
}

int16_t parse_cmd_i2c_tsl2550_pwr_down(char *cmd, char *output, uint16_t len)
{
	uint16_t temp = i2c_tsl2550_pwr(0);
	if (temp == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("tsl2550 down ok")));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("ok")));
#endif
}

int16_t parse_cmd_i2c_tsl2550_mode(char *cmd, char *output, uint16_t len)
{
	while (*cmd == ' ')
		cmd++;
	uint16_t temp = i2c_tsl2550_mode(strcmp(cmd, "std"));
	if (temp == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("tsl2550 mode %s"),(temp==0?"std":"ext")));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%s"),(temp==0?"std":"ext")));
#endif
}

int16_t parse_cmd_i2c_tsl2550_read_lux(char *cmd, char *output, uint16_t len)
{
	uint16_t ret = i2c_tsl2550_read_lux();
	if (ret == 0xffff)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
	if (ret == 0xfffe)
		return ECMD_FINAL(snprintf_P(output, len, PSTR("out of range")));
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("tsl2550 lux %d"), ret));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), ret));
#endif
}

#endif  /* I2C_TSL2550_SUPPORT */

#ifdef I2C_PCA9531_SUPPORT
int16_t
parse_cmd_i2c_pca9531(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	uint8_t period1;
	uint8_t duty1;
	uint8_t period2;
	uint8_t duty2;
	uint8_t firstnibble;
	uint8_t lastnibble;
	sscanf_P(cmd, PSTR("%u %x %x %x %x %x %x"), &adr, &period1, &duty1, &period2, &duty2, &firstnibble, &lastnibble);

#ifdef DEBUG_I2C
	debug_printf("I2C PCA9531 IC %u: pwm1 period %X, duty %X; pwm2 period %X, duty%X; %X %X\n",adr, period1, duty1, period2, duty2, firstnibble, lastnibble);
#endif
	//  i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period, duty, 0x00, 0x40, 0xEF, 0x55);
	i2c_pca9531_set(I2C_SLA_PCA9531 + adr, period1, duty1, period2, duty2, firstnibble, lastnibble);

	return ECMD_FINAL(snprintf_P(output, len, PSTR("pwm ok")));
}

#endif  /* I2C_PCA9531_SUPPORT */

#ifdef I2C_PCF8574X_SUPPORT
int16_t parse_cmd_i2c_pcf8574x_read(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	uint8_t chip;
	sscanf_P(cmd, PSTR("%u %u"), &adr, &chip);

	if (chip == 0)
	{
		adr += I2C_SLA_PCF8574;
	}
	else
	{
		adr += I2C_SLA_PCF8574A;
	}
#ifdef DEBUG_I2C
	debug_printf("I2C PCF8574X IC address 0x%X\n", adr);
#endif
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("pcf8574x read %u %u %X"), adr, chip, i2c_pcf8574x_read(adr)));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), i2c_pcf8574x_read(adr)));
#endif
}

int16_t parse_cmd_i2c_pcf8574x_set(char *cmd, char *output, uint16_t len)
{
	uint8_t adr;
	uint8_t chip;
	uint8_t value;
	sscanf_P(cmd, PSTR("%u %u %x"), &adr, &chip, &value);

	if (chip == 0)
	{
		adr += I2C_SLA_PCF8574;
	}
	else
	{
		adr += I2C_SLA_PCF8574A;
	}
#ifdef DEBUG_I2C
	debug_printf("I2C PCF8574X IC address 0x%X, value:%X\n",adr, value);
#endif
	i2c_pcf8574x_set(adr, value);
#ifdef ECMD_MIRROR_REQUEST
	return ECMD_FINAL(snprintf_P(output, len, PSTR("pcf8574x set %u %u %X"), adr, chip, value));
#else
	return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), value));
#endif
}

#endif  /* I2C_PCF8574X_SUPPORT */

#ifdef I2C_MAX7311_SUPPORT

int16_t parse_cmd_i2c_max7311_setDDRw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hX"), &adr, &data);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_setDDRw(adr, data);
  if (ret == 0) {
    return ECMD_FINAL_OK;
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_setOUTw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hX"), &adr, &data);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_setOUTw(adr, data);
  if (ret == 0) {
    return ECMD_FINAL_OK;
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_getDDRw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getDDRw(adr, &data);
  if (ret == 0) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_getOUTw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getOUTw(adr, &data);
  if (ret == 0) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_getINw(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint16_t data;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu"), &adr);
  if (adr > 0x6F)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_getINw(adr, &data);
  if (ret == 0) {
    return ECMD_FINAL(snprintf_P(output, len, PSTR("%X"), data));
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_set(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t bit;
  uint8_t state;
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &adr, &bit, &state);
  if (adr > 0x6F | bit > 15)
    return ECMD_ERR_PARSE_ERROR;
  ret = i2c_max7311_set(adr, bit, state);
  if (ret == 0) {
    return ECMD_FINAL_OK;
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}

int16_t parse_cmd_i2c_max7311_pulse(char *cmd, char *output, uint16_t len)
{
  uint8_t adr;
  uint8_t bit;
  uint16_t time; 
  uint8_t ret;
  sscanf_P(cmd, PSTR("%hhu %hhu %hu"), &adr, &bit, &time);
  if (adr > 0x6F | bit > 15)
    return ECMD_ERR_PARSE_ERROR;
  if (time > 1000)
    time = 1000;
  ret = i2c_max7311_pulse(adr, bit, time);
  if (ret == 0) {
    return ECMD_FINAL_OK;
  }else{
    return ECMD_FINAL(snprintf_P(output, len, PSTR("no sensor detected")));
  }
}
#endif /* I2C_MAX7311_SUPPORT */
