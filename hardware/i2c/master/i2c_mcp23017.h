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

#ifndef I2C_MCP23017_H
#define I2C_MCP23017_H

#include <stdint.h>
#include <avr/io.h>

#include "config.h"

/*
 * MCP23017 base address 0x20 - 0x27.
 *
 * NOTE:
 * MCP23017 addresses COLLIDE with those of the PCF8574 I/O Expander!!
 *
 */
#define I2C_SLA_MCP23017 0x20
#define I2C_SLA_TOP_MCP23017 (I2C_SLA_MCP23017 + 7)

// CONTROL REGISTER SUMMARY (IOCON.BANK = 0)
//       Register            Address(hex)   bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0      POR/RST value
#define MCP23017_IODIRA         0x00    // IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0              1111 1111
#define MCP23017_IODIRB         0x01    // IO7 IO6 IO5 IO4 IO3 IO2 IO1 IO0              1111 1111
#define MCP23017_IPOLA          0x02    // IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0              0000 0000
#define MCP23017_IPOLB          0x03    // IP7 IP6 IP5 IP4 IP3 IP2 IP1 IP0              0000 0000
#define MCP23017_GPINTENA       0x04    // GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
#define MCP23017_GPINTENB       0x05    // GPINT7 GPINT6 GPINT5 GPINT4 GPINT3 GPINT2 GPINT1 GPINT0 0000 0000
#define MCP23017_DEFVALA        0x06    // DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0      0000 0000
#define MCP23017_DEFVALB        0x07    // DEF7 DEF6 DEF5 DEF4 DEF3 DEF2 DEF1 DEF0      0000 0000
#define MCP23017_INTCONA        0x08    // IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0      0000 0000
#define MCP23017_INTCONB        0x09    // IOC7 IOC6 IOC5 IOC4 IOC3 IOC2 IOC1 IOC0      0000 0000
#define MCP23017_IOCON          0x0A    // BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL —   0000 0000
// Note: There is ONE IOCON register only! 0x0A and 0x0B addresses the same register!
// #define MCP23017_IOCON         0x0B    // BANK MIRROR SEQOP DISSLW HAEN ODR INTPOL —   0000 0000
#define MCP23017_GPPUA          0x0C    // PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0              0000 0000
#define MCP23017_GPPUB          0x0D    // PU7 PU6 PU5 PU4 PU3 PU2 PU1 PU0              0000 0000
#define MCP23017_INTFA          0x0E    // INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO      0000 0000
#define MCP23017_INTFB          0x0F    // INT7 INT6 INT5 INT4 INT3 INT2 INT1 INTO      0000 0000
#define MCP23017_INTCAPA        0x10    // ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0      0000 0000
#define MCP23017_INTCAPB        0x11    // ICP7 ICP6 ICP5 ICP4 ICP3 ICP2 ICP1 ICP0      0000 0000
#define MCP23017_GPIOA          0x12    // GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0              0000 0000
#define MCP23017_GPIOB          0x13    // GP7 GP6 GP5 GP4 GP3 GP2 GP1 GP0              0000 0000
#define MCP23017_OLATA          0x14    // OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0              0000 0000
#define MCP23017_OLATB          0x15    // OL7 OL6 OL5 OL4 OL3 OL2 OL1 OL0              0000 0000


typedef enum _i2c_mcp23017_output_state
{
  ON,
  OFF,
  TOGGLE
} i2c_mcp23017_output_state;


/**
 * Read data from register reg.
 */
uint8_t i2c_mcp23017_read_register(uint8_t address, uint8_t reg,
                                   uint8_t * data);

/**
 * Write data to register reg.
 */
uint8_t i2c_mcp23017_write_register(uint8_t address, uint8_t reg,
                                    uint8_t data);

/**
 * Set or clear pin.
 */
uint8_t i2c_mcp23017_modify_pin(uint8_t address, uint8_t reg, uint8_t * data,
                                uint8_t bit, i2c_mcp23017_output_state state);

/**
 * Toggle pin to create a pulse with duration time.
 */
uint8_t i2c_mcp23017_pulse_pin(uint8_t address, uint8_t reg, uint8_t * data,
                               uint8_t bit, uint16_t time);

#endif /* I2C_MCP23017_H */
