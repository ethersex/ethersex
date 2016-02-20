/*
 *
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
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
 
Instructions for integrating a CAN bus controller MCP2515 a Ethersex system:

a) Hard-/Software
------------------------------------------------------------------------------------
The CAN bus controller can be connected in two ways to the AVR be - 
with MISO, MOSI, SCK and CS are always required::

1. Without INT pin (12) of the MCP2515
This connection saves a port pin on the AVR, but is not performing as permanent
the status of the MCP2515 should be queried via SPI. Therefore I do not recommend.

2. INT PIN (12) of the MCP2515 to a port pin (INT0 / 1/2) of the AVR
In this variant, two software options are now possible:

  2.1 Pollen of MessageBuffer the MCP2515. In this variant is to be checked 
  regularly whether CAN messages have been received. This is detected by a 
  low level on the INT pin (12) of the MCP2515.

  2.2 Interrupt generation upon reception of a CAN message. If you select 
  this setting, be read through an interrupt the incoming CAN messages in 
  a FIFO buffer. For this, the received message can be retrieved.
  

b) Configuration:
------------------------------------------------------------------------------------
1. make menuconfig 
	- "IO" 
	-   "CAN" activate
	-     "MCP2515" activate
	-       "Extended CANID" activate for Extended CAN-Messages
	-       "MCP2515 example" activate for Example
	-       "RX Interrupt" für interrupt reception

2. define CS und INT
In "/pinning/hardware/" in the corresponding * .m4 file of the board used the rows

ifdef(`conf_MCP2515', `
  pin(MCP2515_INT, PD2, INPUT)
  pin(MCP2515_SPI_CS, PB0, OUTPUT)
')

enter append and wired pins.

In interrupt mode in the file "/hardware/can/can_mcp2515.c" in lines from 285 - 300
enter the INT used.


c) Test:
------------------------------------------------------------------------------------
For testing of the CAN bus, the test routines in the file can "can_example.c" can be used.

make menuconfig: "MCP2515 example" activate for Example

This incoming CAN messages are output on the serial interface. Also is every second 
sent a test message.


d) Advanced configuration
------------------------------------------------------------------------------------
The MCP2515 CAN controller has two configurable port pins RX0BF and RX1BF. This can 
be configured from GPIO as output. Given in the "/hardware/can/can_mcp2515.h" the 
entry: MCP2515_RXnBF_OUTPUT set to "1".


I wish you success

Michael
