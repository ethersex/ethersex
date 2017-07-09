/*
 * Copyright (c) 2013-2017 Erik Kunze <ethersex@erik-kunze.de>
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
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"

#include "bt.h"
#include "bt_usart.h"


#ifdef DEBUG_BLUETOOTH
#include "core/debug.h"
#define BTDEBUG(m,...)  debug_printf("BT: " m, ## __VA_ARGS__)
#define BTDEBUG2(m,...) printf_P(PSTR(m), ## __VA_ARGS__)
#else
#define BTDEBUG(...)
#define BTDEBUG2(...)
#endif

#define NELEMS(x)       (sizeof(x)/sizeof(x[0]))


typedef void (*fptr_t) (const char *, va_list);

static int bt_putc(const char, FILE *);
static int bt_getc(FILE *);

FILE bt_stream = FDEV_SETUP_STREAM(bt_putc, bt_getc, _FDEV_SETUP_RW);

static const struct bt_usart_cfg_s
{
  uint32_t baud;
  uint8_t h, l, u2x;
} bt_usart_cfg[3] PROGMEM =
{
  /* virgin module */
  {
    9600,
#undef BAUD
#define BAUD 9600
#include <util/setbaud.h>
    UBRRH_VALUE, UBRRL_VALUE, USE_2X
  },
  /* module in command cmode */
  {
    38400,
#undef BAUD
#define BAUD 38400
#include <util/setbaud.h>
    UBRRH_VALUE, UBRRL_VALUE, USE_2X
  },
  /* module in data mode */
  {
    BLUETOOTH_BAUDRATE,
#undef BAUD
#define BAUD BLUETOOTH_BAUDRATE
#include <util/setbaud.h>
    UBRRH_VALUE, UBRRL_VALUE, USE_2X
  }
};


static int
bt_putc(const char c, FILE * stream)
{
  bt_usart_putchar((uint8_t) c);
  return (int) c;
}


static int
bt_getc(FILE * stream)
{
  return (int) bt_usart_getchar();
}


static void
bt_send(const char *msg, va_list args)
{
#ifdef DEBUG_BLUETOOTH
  printf_P(PSTR("D: BT: "));
  vfprintf(stdout, msg, args);
#endif

  vfprintf(&bt_stream, msg, args);
  fprintf_P(&bt_stream, PSTR("\r\n"));
}


static void
bt_send_P(const char *msg, va_list args)
{
#ifdef DEBUG_BLUETOOTH
  printf_P(PSTR("D: BT: "));
  vfprintf_P(stdout, msg, args);
#endif

  vfprintf_P(&bt_stream, msg, args);
  fprintf_P(&bt_stream, PSTR("\r\n"));
}


static uint8_t
bt_gets_with_timeout(char *buf, uint8_t size)
{
  uint8_t idx;

  for (idx = 0, --size; idx < size; idx++)
  {
    if (!bt_usart_avail())
    {
      _delay_ms(1);
      if (!bt_usart_avail())
        break;
    }
    buf[idx] = bt_usart_getchar();
  }

  buf[idx] = '\0';
#ifdef DEBUG_BLUETOOTH
  printf_P(PSTR("->%s"), buf);
  if (idx == 0)
    putchar('\n');
#endif
  return idx;
}


static uint8_t
bt_send_with_response_internal(fptr_t f,
                               char *response, uint8_t size,
                               const char *msg, va_list args)
{
  PIN_SET(BT_MODULE_KEY);       /* command mode */

  f(msg, args);

  for (uint8_t i = 100; i > 0; i--)
    _delay_ms(1);               /* give the module some time to respond */

  uint8_t res = bt_gets_with_timeout(response, size);

  PIN_CLEAR(BT_MODULE_KEY);     /* data mode */

  return res;
}


uint8_t
bt_send_with_response(char *response, uint8_t size, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  uint8_t res = bt_send_with_response_internal(&bt_send, response, size,
                                               msg, args);
  va_end(args);
  return res;
}


uint8_t
bt_send_with_response_P(char *response, uint8_t size, PGM_P msg, ...)
{
  va_list args;
  va_start(args, msg);
  uint8_t res = bt_send_with_response_internal(&bt_send_P, response, size,
                                               msg, args);
  va_end(args);
  return res;
}


static bool
bt_check_module(void)
{
  char resp[30];

  for (uint8_t i = 0; i < NELEMS(bt_usart_cfg); i++)
  {
    BTDEBUG("trying at %lu baud\n", pgm_read_dword(&bt_usart_cfg[i].baud));

    bt_usart_init(pgm_read_byte(&bt_usart_cfg[i].h),
                  pgm_read_byte(&bt_usart_cfg[i].l),
                  pgm_read_byte(&bt_usart_cfg[i].u2x));

    while (bt_send_with_response_P(resp, sizeof(resp), PSTR("AT")) != 0)
    {
      if (strcmp_P(resp, PSTR("OK\r\n")) == 0)
      {
#ifdef DEBUG_BLUETOOTH
        bt_send_with_response_P(resp, sizeof(resp), PSTR("AT+VERSION?"));
        bt_send_with_response_P(resp, sizeof(resp), PSTR("AT+ADDR?"));
#endif
        return true;
      }
      else if (strcmp_P(resp, PSTR("ERROR:(0)\r\n")) != 0)
      {
        break;
      }
    }
  }
  return false;
}


static bool
bt_configure_module(void)
{
  char resp[30];
  if (bt_send_with_response_P(resp, sizeof(resp),
                              PSTR("AT+NAME=" BLUETOOTH_NAME)) == 0 ||
      strcmp_P(resp, PSTR("OK\r\n")) != 0)
    return false;
  if (bt_send_with_response_P
      (resp, sizeof(resp), PSTR("AT+PSWD=" BLUETOOTH_PIN)) == 0 ||
      strcmp_P(resp, PSTR("OK\r\n")) != 0)
    return false;
  if (bt_send_with_response_P(resp, sizeof(resp),
                              PSTR("AT+UART=%lu,1,0"),
                              BLUETOOTH_BAUDRATE) == 0 ||
      strcmp_P(resp, PSTR("OK\r\n")) != 0)
    return false;
  return true;
}


void
bt_init(void)
{
  BTDEBUG("initializing\n");
  DDR_CONFIG_OUT(BT_MODULE_KEY);
  PIN_CLEAR(BT_MODULE_KEY);

  if (!bt_check_module())
  {
    BTDEBUG("module not responding\n");
    return;
  }

  BTDEBUG("module found\n");
  if (!bt_configure_module())
  {
    BTDEBUG("configuration failed\n");
    return;
  }
  bt_usart_init(pgm_read_byte(&bt_usart_cfg[2].h),
                pgm_read_byte(&bt_usart_cfg[2].l),
                pgm_read_byte(&bt_usart_cfg[2].u2x));
#ifdef ECMD_BLUETOOTH_SUPPORT
  bt_init_finished = true;
#endif
}


/*
  -- Ethersex META --
  header(hardware/radio/bluetooth/bt.h)
  init(bt_init)
*/
