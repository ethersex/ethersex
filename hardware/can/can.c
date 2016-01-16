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

#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "core/debug.h"
#include "can.h"
#include "hardware/can/fifo.h"

// Buffer max. 256, je nach RAM
#define BUFFER_SIZE_RX 16
#define BUFFER_SIZE_TX 16

const uint8_t can_filter[] PROGMEM =
{
#if SUPPORT_EXTENDED_CANID

		// Group 0
		MCP2515_FILTER(0),          	// Filter 0
		MCP2515_FILTER(0),          	// Filter 1

		// Group 1
		MCP2515_FILTER_EXTENDED(0),    // Filter 2
		MCP2515_FILTER_EXTENDED(0),    // Filter 3
		MCP2515_FILTER_EXTENDED(0),    // Filter 4
		MCP2515_FILTER_EXTENDED(0),    // Filter 5

		MCP2515_FILTER(0),          	// Mask 0 (for group 0)
		MCP2515_FILTER_EXTENDED(0),    // Mask 1 (for group 1)

#else
		// Group 0
		MCP2515_FILTER(0),// Filter 0
		MCP2515_FILTER(0),// Filter 1
		// Group 1
		MCP2515_FILTER(0),// Filter 2
		MCP2515_FILTER(0),// Filter 3
		MCP2515_FILTER(0),// Filter 4
		MCP2515_FILTER(0),// Filter 5

		MCP2515_FILTER(0),// Mask 0 (for group 0)
		MCP2515_FILTER(0),// Mask 1 (for group 1)
#endif
	};

// FIFO
#ifdef MCP2515_INTERRUPT
can_t buffer_rx[BUFFER_SIZE_RX];
fifo_t fifo_rx;
#endif
can_t buffer_tx[BUFFER_SIZE_TX];
fifo_t fifo_tx;

// Counter
volatile uint32_t can_messages_received = 0;
volatile uint32_t can_messages_send = 0;

// Max number CAN-Messages in FIFO
volatile uint8_t fifo_max_tx = 0;
volatile uint8_t fifo_max_rx = 0;

/*---------------------------------------------------------------------
 CAN Init
 * --------------------------------------------------------------------*/
void can_init(void)
{
	CANDEBUG("CAN_Init...\n");

#ifdef MCP2515_INTERRUPT
	// FIFO für RX
	fifo_init(&fifo_rx, buffer_rx, BUFFER_SIZE_RX);
#endif
	// FIFO für TX
	fifo_init(&fifo_tx, buffer_tx, BUFFER_SIZE_TX);

	// Initialize MCP2515
	mcp2515_init(BITRATE_125_KBPS);

	// Load filters and masks
	mcp2515_static_filter(can_filter);

#ifdef MCP2515_INTERRUPT

	/* Initialize Interrupt INT0 */

	EICRA &= ~((1 << ISC00) | (1 << ISC01));

	//External Interrupt Mask Register
	EIMSK |= (1 << INT0);

#endif

	CANDEBUG("CAN_Init successful.\n");
}

/*---------------------------------------------------------------------
 CAN Receive Interrupt
 * --------------------------------------------------------------------*/
#ifdef MCP2515_INTERRUPT
ISR( INT0_vect)
{
	uint8_t sreg_local; // SREG
	sreg_local = SREG;

	// MC2515 zieht INT solange auf Low, wie Daten in den Empfangspuffern sind

	// Alle Empfangsbuffer leeren
	while (mcp2515_check_message())
	{
		can_t message;
		if (mcp2515_get_message(&message))
		{
			// Message in FIFO schreiben
			uint8_t ret = fifo_push(&fifo_rx, &message);
			if (!ret)
			{
				// RX FIFO voll!!!
				// Der Application in Möglichkeit geben den RX-FIFO zu leeren,
				// daher nicht gleich beide Empfangsbuffer leeren.
				break;
			}
			if (fifo_rx.count > fifo_max_rx)
			{
				fifo_max_rx = fifo_rx.count;
			}
			can_messages_received++;
		}
	}
	SREG = sreg_local;
}
#endif // MCP2515_INTERRUPT

#ifdef MCP2515_INTERRUPT
/*---------------------------------------------------------------------
 Anzahl Messages im FIFO RX
 * --------------------------------------------------------------------*/
uint8_t can_get_messages_in_rx_fifo(void)
{
	uint8_t count = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = fifo_rx.count;
	}
	return count;
}

/*---------------------------------------------------------------------
 Message im FIFO vorhanden
 * --------------------------------------------------------------------*/
uint8_t can_check_rx_fifo()
{
	uint8_t count = 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = fifo_check(&fifo_rx);
	}
	return count;
}
#endif // MCP2515_INTERRUPT

/*---------------------------------------------------------------------
 CAN Message holen (FIFO oder vom MCP2515)
 * --------------------------------------------------------------------*/
bool can_get_message(can_t *message)
{
	uint8_t ret = 0;

#ifdef MCP2515_INTERRUPT
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (fifo_check(&fifo_rx))
		{
			ret = fifo_pull_nowait(&fifo_rx, message);
		}
	}
#else
	if (mcp2515_check_message())
	{
		ret = mcp2515_get_message(message);
	}
#endif
	return ret;
}

/*---------------------------------------------------------------------
 Anzahl Messages im FIFO TX
 * --------------------------------------------------------------------*/
uint8_t can_get_messages_in_tx_fifo(void)
{
	uint8_t count = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = fifo_tx.count;
	}
	return count;
}

/*---------------------------------------------------------------------
 Gesamtzahl empfangender CAN Messages
 * --------------------------------------------------------------------*/
uint32_t can_get_counter_messages_received()
{
	return can_messages_received;
}

/*---------------------------------------------------------------------
 Gesamtzahl gesendeter CAN Messages
 * --------------------------------------------------------------------*/
uint32_t can_get_counter_messages_send()
{
	return can_messages_send;
}

/*---------------------------------------------------------------------
 Max Anzahl im FIFO TX
 * --------------------------------------------------------------------*/
uint8_t can_get_counter_fifo_max_tx()
{
	return fifo_max_tx;
}

/*---------------------------------------------------------------------
 Max Anzahl im FIFO RX
 * --------------------------------------------------------------------*/
uint8_t can_get_counter_fifo_max_rx()
{
	return fifo_max_rx;
}

/*---------------------------------------------------------------------
 Counter reset
 * --------------------------------------------------------------------*/
void can_reset_counter()
{
	can_messages_received = 0;
	can_messages_send = 0;
	fifo_max_tx = 0;
	fifo_max_rx = 0;
}

/*---------------------------------------------------------------------
 Sendet CAN-Message
 * --------------------------------------------------------------------*/
bool can_send_message(can_t *message)
{
	uint8_t ret;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		ret = fifo_push(&fifo_tx, message);

		if (fifo_tx.count > fifo_max_tx)
		{
			fifo_max_tx = fifo_tx.count;
		}
	}
	return ret;
}

/*---------------------------------------------------------------------
 Versendet CAN Message: Übertragung vom FIFO in den MCP2515
 Diese Funktion muss regelmäßig aufgerufen werden
 * --------------------------------------------------------------------*/
void can_send_tx_fifo()
{
	can_t message;
	uint8_t tx_buffer = 0;
	uint8_t status;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// Message(s) im FIFO?
		while (fifo_check(&fifo_tx))
		{
			// Zustand der drei TX Buffer
			status = mcp2515_get_free_buffer();

			// TX-Buffer 1 frei
			if (bit_is_clear(status, 2))
				tx_buffer = 1;
			// TX-Buffer 2 frei
			else if (bit_is_clear(status, 4))
				tx_buffer = 2;
			// TX-Buffer 3 frei
			else if (bit_is_clear(status, 6))
				tx_buffer = 3;

			if (tx_buffer > 0)
			{
				fifo_pull_nowait(&fifo_tx, &message);
				mcp2515_send_message(&message, tx_buffer);
				can_messages_send++;
				tx_buffer = 0;
			}
			else
			{
				// Alle TX-Buffer voll
				break;
			}
		}
	}
}

/*---------------------------------------------------------------------
 Print CAN Message
 * --------------------------------------------------------------------*/
void can_print_message(const can_t *message)
{
	char data[30] = "\0";
	char rtr[2];

	if (!message->flags.rtr)
	{
		strcpy_P(rtr, PSTR("-"));
		for (uint8_t i = 0; i < message->length; i++)
		{
			sprintf_P(data + (i * 3), PSTR("%02X "), message->data[i]);
		}
	}
	else
	{
		strcpy_P(rtr, PSTR("R"));
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

/*
 -- Ethersex META --
 header(hardware/can/can.h)
 init(can_init)

 */
