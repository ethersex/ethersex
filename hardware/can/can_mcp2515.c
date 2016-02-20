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

#include "can_mcp2515.h"

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdbool.h>
#include "core/spi.h"
#include "core/bool.h"
#include "core/fifo/fifo.h"
#include "config.h"
#include "can_mcp2515_defs.h"

#define FIFO_SIZE_RX 32
#define FIFO_SIZE_TX 16

#ifndef	MCP2515_CLKOUT_PRESCALER
#define	MCP2515_CLKOUT_PRESCALER	0
#endif

#ifndef	MCP2515_CLKOUT_PRESCALER
#error	MCP2515_CLKOUT_PRESCALER not defined!
#elif MCP2515_CLKOUT_PRESCALER == 0
#define	CLKOUT_PRESCALER_	0x0
#elif MCP2515_CLKOUT_PRESCALER == 1
#define	CLKOUT_PRESCALER_	0x4
#elif MCP2515_CLKOUT_PRESCALER == 2
#define	CLKOUT_PRESCALER_	0x5
#elif MCP2515_CLKOUT_PRESCALER == 4
#define	CLKOUT_PRESCALER_	0x6
#elif MCP2515_CLKOUT_PRESCALER == 8
#define	CLKOUT_PRESCALER_	0x7
#else
#error	invaild value of MCP2515_CLKOUT_PRESCALER
#endif

#define cs_low_mcp2515()  PIN_CLEAR(MCP2515_SPI_CS)
#define cs_high_mcp2515() PIN_SET(MCP2515_SPI_CS)

uint8_t mcp2515_send_msg(const can_t *msg, uint8_t tx_buffer);

uint8_t mcp2515_get_msg(can_t *msg);

bool mcp2515_check_free_buffer(void);

uint8_t mcp2515_get_free_buffer(void);

void mcp2515_write_register(uint8_t adress, uint8_t data);

uint8_t mcp2515_read_status(uint8_t type);

uint8_t mcp2515_read_register(uint8_t adress);

void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data);

#if	SUPPORT_EXTENDED_CANID
void mcp2515_write_id(const uint32_t *id, uint8_t extended);

uint8_t mcp2515_read_id(uint32_t *id);
#else
void mcp2515_write_id(const uint16_t *id);

uint8_t mcp2515_read_id(uint16_t *id);
#endif

#ifdef MCP2515_INTERRUPT
static fifo_t fifo_rx;
#endif
static fifo_t fifo_tx;

static volatile uint32_t can_messages_received = 0;
static volatile uint32_t can_messages_send = 0;

const uint8_t _mcp2515_cnf[8][3] PROGMEM =
{
		{ 0x04, 0xb6, 0xe7 },
		{ 0x04, 0xb6, 0xd3 },
		{ 0x04, 0xb6, 0xc7 },
		{ 0x04, 0xb6, 0xc3 },
		{ (1 << PHSEG21),
				(1 << BTLMODE) | (1 << PHSEG11),
				(1 << BRP2) | (1 << BRP1) | (1 << BRP0)
		},
		{ 0x03, 0xac, 0x81 },
		{ 0x03, 0xac, 0x80 },
		{ (1 << PHSEG21), (1 << BTLMODE) | (1 << PHSEG11), 0 } };

bool mcp2515_get_message(can_t * message)
{
	bool ret = false;

#ifdef MCP2515_INTERRUPT
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ret = fifo_get(&fifo_rx, message);
	}
#else
	if (mcp2515_check_message())
	{
		ret = mcp2515_get_msg(message);
	}
#endif
	return ret;
}

uint32_t
mcp2515_get_counter_messages_received()
{
	return can_messages_received;
}

uint32_t
mcp2515_get_counter_messages_send()
{
	return can_messages_send;
}

void
mcp2515_reset_counter()
{
	can_messages_received = 0;
	can_messages_send = 0;
}

bool
mcp2515_send_message(can_t * message)
{
	uint8_t ret;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ret = fifo_put(&fifo_tx, message);
	}
	return ret;
}

void
mcp2515_send_tx_fifo()
{
	can_t message;
	uint8_t tx_buffer = 0;
	uint8_t status;
	bool ret = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		while (!fifo_empty(&fifo_tx))
		{
			status = mcp2515_get_free_buffer();
			if (bit_is_clear(status, 2))
				tx_buffer = 1;
			else if (bit_is_clear(status, 4))
				tx_buffer = 2;
			else if (bit_is_clear(status, 6))
				tx_buffer = 3;
			if (tx_buffer > 0)
			{
				ret = fifo_get(&fifo_tx, &message);
				if (ret)
				{
					mcp2515_send_msg(&message, tx_buffer);
					can_messages_send++;
					tx_buffer = 0;
				}
			}
			else
			{
				break;
			}
		}
	}
}

#ifdef DEBUG_MCP2515
void
mcp2515_print_message(const can_t * message)
{
	char data[30] = "\0";
	char rtr[2];

	if (!message->flags.rtr)
	{
		strcpy_P(rtr, PSTR ("-"));
		for (uint8_t i = 0; i < message->length; i++)
		{
			sprintf_P(data + (i * 3), PSTR("%02X "), message->data[i]);
		}
	}
	else
	{
		strcpy_P(rtr, PSTR ("R"));
		sprintf_P(data, PSTR("remote request"));
	}

#if SUPPORT_EXTENDED_CANID
	if (message->flags.extended)
	{
		printf("%08lX [%sE][%u] %s\n", message->id, rtr, message->length, data);
	}
	else
	{
		printf("     %03lX [%s-][%u] %s\n", message->id, rtr, message->length,
				data);
	}
#else
	printf("%03X [%s-][%u] %s\n", message->id, rtr, message->length, data);
#endif
}
#endif

bool
mcp2515_init(can_bitrate_t bitrate)
{
	uint8_t sreg = SREG;
	cli();
	CANDEBUG("CAN_Init...\n");

#ifdef MCP2515_INTERRUPT
	fifo_init(&fifo_rx, FIFO_SIZE_RX, sizeof(can_t));
#endif
	fifo_init(&fifo_tx, FIFO_SIZE_TX, sizeof(can_t));
	cs_low_mcp2515();
	spi_send(SPI_RESET);
	_delay_ms(1);
	cs_high_mcp2515();
	_delay_ms(10);
	cs_low_mcp2515();
	spi_send(SPI_WRITE);
	spi_send(CNF3);
	for (uint8_t i = 0; i < 3; i++)
	{
		spi_send(pgm_read_byte(&_mcp2515_cnf[bitrate][i]));
	}
	spi_send((1 << RX1IE) | (1 << RX0IE));
	cs_high_mcp2515();
	mcp2515_write_register(TXRTSCTRL, 0);

#if MCP2515_RXnBF_OUTPUT
	mcp2515_write_register(BFPCTRL, (1 << B0BFE) | (1 << B1BFE));
#else
	mcp2515_write_register(BFPCTRL, 0);
#endif

	bool error = false;
	if (mcp2515_read_register(CNF2) != pgm_read_byte(&_mcp2515_cnf[bitrate][1]))
	{
		error = true;
	}

	mcp2515_write_register(CANCTRL, CLKOUT_PRESCALER_);

	if (error)
	{
		SREG = sreg;
		return false;
	}
	else
	{
		while ((mcp2515_read_register(CANSTAT) & 0xe0) != 0)
			;
	}
#ifdef MCP2515_INTERRUPT
	/* Initialize Interrupt INT0 */
	EICRA &= ~((1 << ISC00) | (1 << ISC01));

	//External Interrupt Mask Register
	EIMSK |= (1 << INT0);
#endif
	CANDEBUG("CAN_Init successful.\n");
	SREG = sreg;
	return true;
}

#ifdef MCP2515_INTERRUPT
ISR( INT0_vect)
{
	can_t message;
	bool ret;
	uint8_t sreg_local;

	sreg_local = SREG;

	while (mcp2515_check_message())
	{
		if (mcp2515_get_msg(&message))
		{
			ret = fifo_put(&fifo_rx, &message);
			if (!ret)
			{
				// RX buffer full
				break;
			}
			can_messages_received++;
		}
	}
	SREG = sreg_local;
}

bool
mcp2515_rx_fifo_empty()
{
	bool stat;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		stat = fifo_empty(&fifo_rx);
	}
	return stat;
}
#endif // MCP2515_INTERRUPT

uint8_t
mcp2515_send_msg(const can_t *msg, uint8_t tx_buffer)
{
	uint8_t address = 0;
	uint8_t sreg = SREG;

	cli();
	if (tx_buffer == 1)
	{
		address = 0x00;
	}
	else if (tx_buffer == 2)
	{
		address = 0x02;
	}
	else if (tx_buffer == 3)
	{
		address = 0x04;
	}
	cs_low_mcp2515();
	spi_send(SPI_WRITE_TX | address);

#if SUPPORT_EXTENDED_CANID
	mcp2515_write_id(&msg->id, msg->flags.extended);
#else
	mcp2515_write_id(&msg->id);
#endif

	uint8_t length = msg->length & 0x0f;
	if (msg->flags.rtr)
	{
		spi_send((1 << RTR) | length);
	}
	else
	{
		spi_send(length);

		for (uint8_t i = 0; i < length; i++)
		{
			spi_send(msg->data[i]);
		}
	}
	cs_high_mcp2515();
	_delay_us(1);
	cs_low_mcp2515();
	address = (address == 0) ? 1 : address;
	spi_send(SPI_RTS | address);
	cs_high_mcp2515();
	SREG = sreg;
	return address;
}

uint8_t
mcp2515_get_msg(can_t *msg)
{
	uint8_t addr;
	uint8_t sreg = SREG;

	cli();

	uint8_t status = mcp2515_read_status(SPI_RX_STATUS);

	if (bit_is_set(status, 6))
	{
		addr = SPI_READ_RX;
	}
	else if (bit_is_set(status, 7))
	{
		addr = SPI_READ_RX | 0x04;
	}
	else
	{
		SREG = sreg;
		return 0;
	}

	cs_low_mcp2515();
	spi_send(addr);

	uint8_t tmp = mcp2515_read_id(&msg->id);

#if SUPPORT_EXTENDED_CANID
	msg->flags.extended = tmp & 0x01;
#else
	if (tmp & 0x01)
	{
		cs_high_mcp2515();;
		if (bit_is_set(status, 6))
		mcp2515_bit_modify(CANINTF, (1 << RX0IF), 0);
		else
		mcp2515_bit_modify(CANINTF, (1 << RX1IF), 0);
		SREG = sreg;
		return 0;
	}
#endif

	uint8_t length = spi_send(0xff);

	msg->flags.rtr = (bit_is_set(status, 3)) ? 1 : 0;
	length &= 0x0f;
	msg->length = length;
	for (uint8_t i = 0; i < length; i++)
	{
		msg->data[i] = spi_send(0xff);
	}
	cs_high_mcp2515();

	if (bit_is_set(status, 6))
		mcp2515_bit_modify(CANINTF, (1 << RX0IF), 0);
	else
		mcp2515_bit_modify(CANINTF, (1 << RX1IF), 0);

	SREG = sreg;
	return (status & 0x07) + 1;
}

bool
mcp2515_check_message(void)
{
#if defined(HAVE_MCP2515_INT)
	return (!(PIN_HIGH(MCP2515_INT)) ? true : false);
#else
	return ((mcp2515_read_status(SPI_RX_STATUS) & 0xC0) ? true : false);
#endif

}

void
mcp2515_static_filter(const uint8_t *filter)
{
	uint8_t sreg = SREG;

	cli();
	mcp2515_bit_modify(CANCTRL, 0xe0, (1 << REQOP2));
	while ((mcp2515_read_register(CANSTAT) & 0xe0) != (1 << REQOP2))
		;

	mcp2515_write_register(RXB0CTRL, (1 << BUKT));
	mcp2515_write_register(RXB1CTRL, 0);

	uint8_t i, j;
	for (i = 0; i < 0x30; i += 0x10)
	{
		cs_low_mcp2515();
		spi_send(SPI_WRITE);
		spi_send(i);
		for (j = 0; j < 12; j++)
		{
			if (i == 0x20 && j >= 0x08)
				break;

			spi_send(pgm_read_byte(filter++));
		}
		cs_high_mcp2515();
	}
	mcp2515_bit_modify(CANCTRL, 0xe0, 0);
	SREG = sreg;
}

bool
mcp2515_check_free_buffer(void)
{
	uint8_t sreg = SREG;

	cli();
	uint8_t status = mcp2515_read_status(SPI_READ_STATUS);
	if ((status & 0x54) == 0x54)
	{
		return false;
		SREG = sreg;
	}
	else
	{
		SREG = sreg;
		return true;
	}
}

uint8_t
mcp2515_get_free_buffer(void)
{
	uint8_t sreg = SREG;
	uint8_t status;

	cli();
	status = mcp2515_read_status(SPI_READ_STATUS);
	SREG = sreg;
	return status;
}

can_error_register_t
mcp2515_read_error_register(void)
{
	uint8_t sreg = SREG;
	can_error_register_t error;

	cli();
	error.tx = mcp2515_read_register(TEC);
	error.rx = mcp2515_read_register(REC);
	SREG = sreg;
	return error;
}

uint8_t
mcp2515_read_error_flags(void)
{
	uint8_t sreg = SREG;
	uint8_t errorflags;

	cli();
	errorflags = mcp2515_read_register(EFLG);
	SREG = sreg;
	return errorflags;
}

#if	SUPPORT_EXTENDED_CANID

uint8_t
mcp2515_read_id(uint32_t *id)
{
	uint8_t first;
	uint8_t tmp;

	first = spi_send(0xff);
	tmp = spi_send(0xff);

	if (tmp & (1 << IDE))
	{
		*((uint16_t *) id + 1) = (uint16_t) first << 5;
		*((uint8_t *) id + 1) = spi_send(0xff);
		*((uint8_t *) id + 2) |= (tmp >> 3) & 0x1C;
		*((uint8_t *) id + 2) |= tmp & 0x03;
		*((uint8_t *) id) = spi_send(0xff);
		return TRUE;
	}
	else
	{
		spi_send(0xff);
		*((uint8_t *) id + 3) = 0;
		*((uint8_t *) id + 2) = 0;
		*((uint16_t *) id) = (uint16_t) first << 3;

		spi_send(0xff);
		*((uint8_t *) id) |= tmp >> 5;
		return FALSE;
	}
}

#else

uint8_t
mcp2515_read_id(uint16_t *id)
{
	uint8_t first;
	uint8_t tmp;

	first = spi_send(0xff);
	tmp = spi_send(0xff);

	if (tmp & (1 << IDE))
	{
		spi_send(0xff);
		spi_send(0xff);

		return 1;
	}
	else
	{
		spi_send(0xff);
		*id = (uint16_t) first << 3;
		spi_send(0xff);
		*((uint8_t *) id) |= tmp >> 5;

		if (tmp & (1 << SRR))
		return 2;
		else
		return 0;
	}
}

#endif

void
mcp2515_set_mode(can_mode_t mode)
{
	uint8_t reg = 0;

	if (mode == LISTEN_ONLY_MODE)
	{
		reg = (1 << REQOP1) | (1 << REQOP0);
	}
	else if (mode == LOOPBACK_MODE)
	{
		reg = (1 << REQOP1);
	}
	else if (mode == SLEEP_MODE)
	{
		reg = (1 << REQOP0);
	}

	mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0),
			reg);
	while ((mcp2515_read_register(CANSTAT) & 0xe0) != reg)
		;
}

#if MCP2515_RXnBF_OUTPUT
void
mcp2515_rx0bf(uint8_t level)
{
	uint8_t sreg = SREG;

	cli();
	if (level)
	mcp2515_bit_modify(BFPCTRL, (1 << B0BFS), (1 << B0BFS));
	else
	mcp2515_bit_modify(BFPCTRL, (1 << B0BFS), 0);
	SREG = sreg;
}

void
mcp2515_rx1bf(uint8_t level)
{
	uint8_t sreg = SREG;

	cli();
	if (level)
	mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), (1 << B1BFS));
	else
	mcp2515_bit_modify(BFPCTRL, (1 << B1BFS), 0);
	SREG = sreg;
}
#endif

#if SUPPORT_EXTENDED_CANID
void
mcp2515_write_id(const uint32_t *id, uint8_t extended)
{
	uint8_t tmp;

	if (extended)
	{
		spi_send(*((uint16_t *) id + 1) >> 5);

		tmp = (*((uint8_t *) id + 2) << 3) & 0xe0;
		tmp |= (1 << IDE);
		tmp |= (*((uint8_t *) id + 2)) & 0x03;

		spi_send(tmp);
		spi_send(*((uint8_t *) id + 1));
		spi_send(*((uint8_t *) id));
	}
	else
	{
		spi_send(*((uint16_t *) id) >> 3);

		tmp = *((uint8_t *) id) << 5;

		spi_send(tmp);
		spi_send(0);
		spi_send(0);
	}
}

#else

void
mcp2515_write_id(const uint16_t *id)
{
	uint8_t tmp;

	spi_send(*id >> 3);
	tmp = *((uint8_t *) id) << 5;

	spi_send(tmp);
	spi_send(0);
	spi_send(0);
}

#endif

void
mcp2515_write_register(uint8_t adress, uint8_t data)
{
	cs_low_mcp2515();
	spi_send(SPI_WRITE);
	spi_send(adress);
	spi_send(data);
	cs_high_mcp2515();
}

uint8_t
mcp2515_read_register(uint8_t adress)
{
	uint8_t data;

	cs_low_mcp2515();
	spi_send(SPI_READ);
	spi_send(adress);
	data = spi_send(0xff);
	cs_high_mcp2515();
	return data;
}

void
mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
	cs_low_mcp2515();
	spi_send(SPI_BIT_MODIFY);
	spi_send(adress);
	spi_send(mask);
	spi_send(data);
	cs_high_mcp2515();
}

uint8_t
mcp2515_read_status(uint8_t type)
{
	uint8_t data;
	cs_low_mcp2515();
	spi_send(type);
	data = spi_send(0xff);
	cs_high_mcp2515();
	return data;
}
