/*
*
* Copyright (c) 2011 by warhog <warhog@gmx.de>
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

#include <util/delay.h>
#include "lome6_ecmd.h"

/*
* ECMD handler: "lome6 reset"
*
* stimulate reset relais for 5ms
*/
int16_t parse_cmd_lome6_reset(char *cmd, char *output, uint16_t len) {

	PIN_SET(RELAIS_RESET);
	_delay_ms(CONF_TIME2PRESS_RESET);
	PIN_CLEAR(RELAIS_RESET);

	return ECMD_FINAL_OK;

}



/*
* ECMD handler: "lome6 power"
*
* stimulate power relais for 40ms or 3000ms
*/
int16_t parse_cmd_lome6_power(char *cmd, char *output, uint16_t len) {

	uint8_t iType = 0;

	if (strcmp_P(cmd, PSTR(" long")) == 0)
		iType = 1;

	PIN_SET(RELAIS_POWER);
	if (iType == 1) {

		// wait 3 seconds, then clear pin (see lome6.c -> lome6_countdowntimer())
		// this is needed because the watchdog will reset the controller when waiting longer than 2sec.
		iCountdownTimer = CONF_TIME2PRESS_POWERL;

	} else {

		_delay_ms(CONF_TIME2PRESS_POWER);
		PIN_CLEAR(RELAIS_POWER);

	}

	return ECMD_FINAL_OK;

}


/*
* ECMD handler: "lome6 state"
*
* get state
*/
int16_t parse_cmd_lome6_state(char *cmd, char *output, uint16_t len) {

	if (PIN_HIGH(POWER_STATE)) {

		len = 3;
		memcpy(output, "off", len);

	} else {

		len = 2;
		memcpy(output, "on", len);

	}

	return ECMD_FINAL(len);

}


/*
* ECMD handler: "lome6 set_t"
*
* set temperature
*/
int16_t parse_cmd_lome6_set_t(char *cmd, char *output, uint16_t len) {

	uint16_t temp = 0;
	char type[20] = { 0 };

	uint8_t ret = sscanf_P(cmd, PSTR("%s %d"), type, &temp);

	// check if 2 parameters are given
	if (ret == 2) {

		if (strcmp(type, "cpu") == 0) {

			iTemperatureCPU = temp;

		} else {

			iTemperatureSB = temp;

		}

		return ECMD_FINAL_OK;

	}

	return ECMD_ERR_PARSE_ERROR;

}



/*
* ECMD handler: "lome6 get_t"
*
* get temperature
*/
int16_t parse_cmd_lome6_get_t(char *cmd, char *output, uint16_t len) {

	// if machine is down, set the external temperatures to zero
	if (PIN_HIGH(POWER_STATE)) {

		iTemperatureSB = 0;
		iTemperatureCPU = 0;

	}

	if (strcmp_P(cmd, PSTR(" cpu")) == 0)
		len = snprintf_P(output, len, PSTR("%d.%d"), iTemperatureCPU / 10, iTemperatureCPU % 10);
	else if (strcmp_P(cmd, PSTR(" sb")) == 0)
		len = snprintf_P(output, len, PSTR("%d.%d"), iTemperatureSB / 10, iTemperatureSB % 10);
#ifdef LOME6_ONEWIRE_SUPPORT
	else if (strcmp_P(cmd, PSTR(" air")) == 0)
		len = snprintf_P(output, len, PSTR("%d.%d"), iTemperatureAIR / 10, iTemperatureAIR % 10);
	else if (strcmp_P(cmd, PSTR(" psu")) == 0)
		len = snprintf_P(output, len, PSTR("%d.%d"), iTemperaturePSU / 10, iTemperaturePSU % 10);
	else if (strcmp_P(cmd, PSTR(" ram")) == 0)
		len = snprintf_P(output, len, PSTR("%d.%d"), iTemperatureRAM / 10, iTemperatureRAM % 10);
#endif
	else return ECMD_ERR_PARSE_ERROR;

	return ECMD_FINAL(len);

}



/*
* ECMD handler: "lome6 uptime"
*
* set/get server uptime
*/
int16_t parse_cmd_lome6_uptime(char *cmd, char *output, uint16_t len) {

	// if machine is down, set the uptime to zero
	if (PIN_HIGH(POWER_STATE))
		iUptime = 0;

	uint32_t uptime = 0;
	uint8_t ret = sscanf_P(cmd, PSTR("%ld"), &uptime);

	// if 1 parameter is given -> set uptime
	if (ret == 1) {

		iUptime = uptime;
		return ECMD_FINAL_OK;

	}
	len = snprintf_P(output, len, PSTR("%ld"), iUptime);
	return ECMD_FINAL(len);

}


/*
  -- Ethersex META --
  block(lome6)
  ecmd_feature(lome6_reset, "lome6 reset",, press reset command)
  ecmd_feature(lome6_power, "lome6 power", TYPE, press power command)
  ecmd_feature(lome6_state, "lome6 state",, get state command)
  ecmd_feature(lome6_set_t, "lome6 set_t", TYPE TEMPERATURE, set temperature command)
  ecmd_feature(lome6_get_t, "lome6 get_t", TYPE, get temperature command)
  ecmd_feature(lome6_uptime, "lome6 uptime", UPTIME, set/get uptime command)
*/
