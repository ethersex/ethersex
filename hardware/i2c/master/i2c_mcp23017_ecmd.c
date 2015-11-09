/*
 *
 * Copyright (c) 2015 Michael Brakemeier <michael@brakemeier.de>
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

#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "config.h"

#include "core/debug.h"
#include "protocols/ecmd/ecmd-base.h"

#include "i2c_mcp23017.h"

/*
 * Prototypes
 */
int16_t parse_cmd_i2c_mcp23017_setreg(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_i2c_mcp23017_getreg(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_i2c_mcp23017_set_iodir(char *cmd, char *output,
                                         uint16_t len);
int16_t parse_cmd_i2c_mcp23017_get_iodir(char *cmd, char *output,
                                         uint16_t len);
int16_t parse_cmd_i2c_mcp23017_set_olat(char *cmd, char *output,
                                        uint16_t len);
int16_t parse_cmd_i2c_mcp23017_get_olat(char *cmd, char *output,
                                        uint16_t len);
int16_t parse_cmd_i2c_mcp23017_get_port(char *cmd, char *output,
                                        uint16_t len);
int16_t parse_cmd_i2c_mcp23017_set_pin(char *cmd, char *output, uint16_t len);
int16_t parse_cmd_i2c_mcp23017_clear_pin(char *cmd, char *output,
                                         uint16_t len);
int16_t parse_cmd_i2c_mcp23017_toggle_pin(char *cmd, char *output,
                                          uint16_t len);
int16_t parse_cmd_i2c_mcp23017_pulse_pin(char *cmd, char *output,
                                         uint16_t len);

#ifdef ECMD_MIRROR_REQUEST
static int16_t cmd_i2c_mcp23017_read_register(char *cmd, char *output,
                                              uint16_t len, uint8_t reg_A,
                                              uint8_t reg_B,
                                              const char *ecmd_mirror);
static int16_t cmd_i2c_mcp23017_write_register(char *cmd, char *output,
                                               uint16_t len, uint8_t reg_A,
                                               uint8_t reg_B,
                                               const char *ecmd_mirror);
#else
static int16_t cmd_i2c_mcp23017_read_register(char *cmd, char *output,
                                              uint16_t len, uint8_t reg_A,
                                              uint8_t reg_B);
static int16_t cmd_i2c_mcp23017_write_register(char *cmd, char *output,
                                               uint16_t len, uint8_t reg_A,
                                               uint8_t reg_B);
#endif

static int16_t cmd_i2c_mcp23017_modify_pin(char *cmd, char *output,
                                           uint16_t len,
                                           i2c_mcp23017_output_state state);


int16_t
parse_cmd_i2c_mcp23017_setreg(char *cmd, char *output, uint16_t len)
{
  uint8_t address;
  uint8_t reg;
  uint8_t data;

  sscanf_P(cmd, PSTR("%hhu %hhu %hhx"), &address, &reg, &data);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017) ||
      (reg > 0x15))
    return ECMD_ERR_PARSE_ERROR;

  if (i2c_mcp23017_write_register(address, reg, data) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 setreg %u %u 0x%X"), address,
                  reg, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), data));
#endif
  }

  return ECMD_ERR_WRITE_ERROR;
}


int16_t
parse_cmd_i2c_mcp23017_getreg(char *cmd, char *output, uint16_t len)
{
  uint8_t address;
  uint8_t reg;
  uint8_t data;

  sscanf_P(cmd, PSTR("%hhu %hhu"), &address, &reg);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017) ||
      (reg > 0x15))
    return ECMD_ERR_PARSE_ERROR;

  if (i2c_mcp23017_read_register(address, reg, &data) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 getreg %u %u 0x%X"), address,
                  reg, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), data));
#endif
  }

  return ECMD_ERR_READ_ERROR;
}


#ifdef ECMD_MIRROR_REQUEST
static int16_t
cmd_i2c_mcp23017_read_register(char *cmd, char *output, uint16_t len,
                               uint8_t reg_A, uint8_t reg_B,
                               const char *ecmd_mirror)
#else
static int16_t
cmd_i2c_mcp23017_read_register(char *cmd, char *output, uint16_t len,
                               uint8_t reg_A, uint8_t reg_B)
#endif
{
  uint8_t address;
  uint8_t reg;
  uint8_t data;
  char port;

  sscanf_P(cmd, PSTR("%hhu %c"), &address, &port);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017))
    return ECMD_ERR_PARSE_ERROR;

  switch (port & 0xDF)
  {
    case 'A':
      reg = reg_A;
      break;

    case 'B':
      reg = reg_B;
      break;

    default:
      return ECMD_ERR_PARSE_ERROR;
  }

  if (i2c_mcp23017_read_register(address, reg, &data) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 %S %u %c 0x%X"), ecmd_mirror,
                  address, port, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), data));
#endif
  }

  return ECMD_ERR_READ_ERROR;
}


#ifdef ECMD_MIRROR_REQUEST
static int16_t
cmd_i2c_mcp23017_write_register(char *cmd, char *output, uint16_t len,
                                uint8_t reg_A, uint8_t reg_B,
                                const char *ecmd_mirror)
#else
static int16_t
cmd_i2c_mcp23017_write_register(char *cmd, char *output, uint16_t len,
                                uint8_t reg_A, uint8_t reg_B)
#endif
{
  uint8_t address;
  uint8_t reg;
  uint8_t data;
  char port;

  sscanf_P(cmd, PSTR("%hhu %c %hhx"), &address, &port, &data);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017))
    return ECMD_ERR_PARSE_ERROR;

  switch (port & 0xDF)
  {
    case 'A':
      reg = reg_A;
      break;

    case 'B':
      reg = reg_B;
      break;

    default:
      return ECMD_ERR_PARSE_ERROR;
  }

  if (i2c_mcp23017_write_register(address, reg, data) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 %S %u %c 0x%X"), ecmd_mirror,
                  address, port, data));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), data));
#endif
  }

  return ECMD_ERR_WRITE_ERROR;
}


int16_t
parse_cmd_i2c_mcp23017_set_iodir(char *cmd, char *output, uint16_t len)
{
#ifdef ECMD_MIRROR_REQUEST
  return cmd_i2c_mcp23017_write_register(cmd, output, len, MCP23017_IODIRA,
                                         MCP23017_IODIRB, PSTR("set iodir"));
#else
  return cmd_i2c_mcp23017_write_register(cmd, output, len, MCP23017_IODIRA,
                                         MCP23017_IODIRB);
#endif
}


int16_t
parse_cmd_i2c_mcp23017_get_iodir(char *cmd, char *output, uint16_t len)
{
#ifdef ECMD_MIRROR_REQUEST
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_IODIRA,
                                        MCP23017_IODIRB, PSTR("get iodir"));
#else
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_IODIRA,
                                        MCP23017_IODIRB);
#endif
}


int16_t
parse_cmd_i2c_mcp23017_set_olat(char *cmd, char *output, uint16_t len)
{
#ifdef ECMD_MIRROR_REQUEST
  return cmd_i2c_mcp23017_write_register(cmd, output, len, MCP23017_OLATA,
                                         MCP23017_OLATB, PSTR("set olat"));
#else
  return cmd_i2c_mcp23017_write_register(cmd, output, len, MCP23017_OLATA,
                                         MCP23017_OLATB);
#endif
}


int16_t
parse_cmd_i2c_mcp23017_get_olat(char *cmd, char *output, uint16_t len)
{
#ifdef ECMD_MIRROR_REQUEST
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_OLATA,
                                        MCP23017_OLATB, PSTR("get olat");
#else
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_OLATA,
                                        MCP23017_OLATB);
#endif
}


int16_t
parse_cmd_i2c_mcp23017_get_port(char *cmd, char *output, uint16_t len)
{
#ifdef ECMD_MIRROR_REQUEST
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_GPIOA,
                                        MCP23017_GPIOB, PSTR("get port");
#else
  return cmd_i2c_mcp23017_read_register(cmd, output, len, MCP23017_GPIOA,
                                        MCP23017_GPIOB);
#endif
}


static int16_t
cmd_i2c_mcp23017_modify_pin(char *cmd, char *output, uint16_t len,
                            i2c_mcp23017_output_state state)
{
  uint8_t address;
  uint8_t reg;
  uint8_t bit;
  char port;

  sscanf_P(cmd, PSTR("%hhu %c %hhu"), &address, &port, &bit);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017) ||
      (bit > 7))
    return ECMD_ERR_PARSE_ERROR;

  switch (port & 0xDF)
  {
    case 'A':
      reg = MCP23017_OLATA;
      break;

    case 'B':
      reg = MCP23017_OLATB;
      break;

    default:
      return ECMD_ERR_PARSE_ERROR;
  }

  uint8_t tmp;
  if (i2c_mcp23017_modify_pin(address, reg, &tmp, bit, state) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 %S pin %u %c 0x%X"),
                  (state ? PSTR("set") : PSTR("clear")), address, port, tmp));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), tmp));
#endif
  }

  return ECMD_ERR_WRITE_ERROR;
}


int16_t
parse_cmd_i2c_mcp23017_set_pin(char *cmd, char *output, uint16_t len)
{
  return cmd_i2c_mcp23017_modify_pin(cmd, output, len, ON);
}


int16_t
parse_cmd_i2c_mcp23017_clear_pin(char *cmd, char *output, uint16_t len)
{
  return cmd_i2c_mcp23017_modify_pin(cmd, output, len, OFF);
}


int16_t
parse_cmd_i2c_mcp23017_toggle_pin(char *cmd, char *output, uint16_t len)
{
  return cmd_i2c_mcp23017_modify_pin(cmd, output, len, TOGGLE);
}


int16_t
parse_cmd_i2c_mcp23017_pulse_pin(char *cmd, char *output, uint16_t len)
{
  uint8_t address;
  uint8_t reg;
  uint8_t bit;
  uint16_t time;
  char port;

  sscanf_P(cmd, PSTR("%hhu %c %hhu %hu"), &address, &port, &bit, &time);

  if ((address < I2C_SLA_MCP23017) || (address > I2C_SLA_TOP_MCP23017) ||
      (bit > 7))
    return ECMD_ERR_PARSE_ERROR;

  switch (port & 0xDF)
  {
    case 'A':
      reg = MCP23017_OLATA;
      break;

    case 'B':
      reg = MCP23017_OLATB;
      break;

    default:
      return ECMD_ERR_PARSE_ERROR;
  }

  uint8_t tmp;
  if (i2c_mcp23017_pulse_pin(address, reg, &tmp, bit, time) > 0)
  {
#ifdef ECMD_MIRROR_REQUEST
    return
      ECMD_FINAL(snprintf_P
                 (output, len, PSTR("mcp23017 pulse pin %u %c %u %u 0x%X"),
                  address, port, bit, time, tmp));
#else
    return ECMD_FINAL(snprintf_P(output, len, PSTR("0x%X"), tmp));
#endif
  }

  return ECMD_ERR_WRITE_ERROR;
}

/*
  -- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(i2c_mcp23017_setreg, "mcp23017 setreg", ADDR REGADDR VALUE, Set Register REGADDR (VALUE as hex))
  ecmd_feature(i2c_mcp23017_getreg, "mcp23017 getreg", ADDR REGADDR, Get Register REGADDR)
  ecmd_feature(i2c_mcp23017_set_iodir, "mcp23017 set iodir", ADDR PORT VALUE, Set I/O Direction Register for PORT A or B (VALUE as hex))
  ecmd_feature(i2c_mcp23017_get_iodir, "mcp23017 get iodir", ADDR PORT, Get I/O Direction Register for PORT A or B)
  ecmd_feature(i2c_mcp23017_set_olat, "mcp23017 set olat", ADDR PORT VALUE, Set Output Latch Register for PORT A or B (VALUE as hex))
  ecmd_feature(i2c_mcp23017_get_olat, "mcp23017 get olat", ADDR PORT, Get Output Latch Register for PORT A or B)
  ecmd_feature(i2c_mcp23017_get_port, "mcp23017 get port", ADDR PORT, Get Port Register (i.e. Port Pin State) for PORT A or B )
  ecmd_feature(i2c_mcp23017_set_pin, "mcp23017 set pin", ADDR PORT BIT, Set Port BIT for PORT A or B )
  ecmd_feature(i2c_mcp23017_clear_pin, "mcp23017 clear pin", ADDR PORT BIT, Clear Port BIT for PORT A or B )
  ecmd_feature(i2c_mcp23017_toggle_pin, "mcp23017 toggle pin", ADDR PORT BIT, Toggle Port BIT for PORT A or B )
  ecmd_feature(i2c_mcp23017_pulse_pin, "mcp23017 pulse pin", ADDR PORT BIT TIME, Toggle-Pulse Port BIT for PORT A or B for TIME ms)
*/
