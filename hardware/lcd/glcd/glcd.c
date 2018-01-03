/*
 * Copyright (c) 2017 Erik Kunze <ethersex@erik-kunze.de>
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
#include <stdio.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "core/debug.h"

#include "glcd.h"


#ifdef DEBUG
#define DEBUG_LCD(fnc, s, msg...) printf_P(PSTR("D: LCD: " #fnc ": " s), ## msg)
#else
#define DEBUG_LCD(fnc, s, msg...)
#endif

#ifdef GLCD_SPI_SUPPORT
/* Prologue/epilogue macros, disabling/enabling interrupts. */
#ifdef RFM12_IP_SUPPORT
/* RFM12 uses interrupts which do SPI interaction, therefore
   we have to disable interrupts if support is enabled */
#define GLCD_SPI_TRANSACTION
#endif

#ifdef HAVE_GLCD_RESET
#define glcd_rst_configure()  DDR_CONFIG_OUT(GLCD_RESET);
#define glcd_rst_low()        PIN_CLEAR(GLCD_RESET)
#define glcd_rst_high()       PIN_SET(GLCD_RESET)
#endif
#define glcd_cs_configure()   DDR_CONFIG_OUT(GLCD_CS); PIN_SET(GLCD_CS);
#define glcd_cs_low()         PIN_CLEAR(GLCD_CS)
#define glcd_cs_high()        PIN_SET(GLCD_CS)
#define glcd_mode_configure() DDR_CONFIG_OUT(GLCD_CD); PIN_CLEAR(GLCD_CD);
#define glcd_mode_cmd()       PIN_CLEAR(GLCD_CD)
#define glcd_mode_data()      PIN_SET(GLCD_CD)
#endif /* GLCD_SPI_SUPPORT */


static int glcd_putc(char c, FILE * stream);

glcd_t glcd;
FILE glcd_stream = FDEV_SETUP_STREAM(glcd_putc, NULL, _FDEV_SETUP_WRITE);

static int16_t tx, ty;
static uint8_t tdir;

#ifdef GLCD_MONO_SUPPORT
#include "glcd_mono.c"
#endif

#ifdef GLCD_COLOR_SUPPORT
#include "glcd_color.c"
#endif


void
glcd_set_print_pos(uint16_t x, uint16_t y)
{
  tx = x;
  ty = y;
}

void
glcd_set_print_dir(uint8_t dir)
{
  tdir = dir;
}

uint16_t
glcd_get_x(void)
{
  return tx;
}

uint16_t
glcd_get_y(void)
{
  return ty;
}

static int
glcd_putc(char c, FILE * stream)
{
#ifdef GLCD_MONO_SUPPORT
  int16_t delta = glcd_draw_glyph(tx, ty, c);
#else
  int16_t delta = glcd_draw_glyph(tx, ty, tdir, c);
#endif

  switch (tdir)
  {
    default:
    case GLCD_DIR_LTR:
      tx += delta;
      break;
    case GLCD_DIR_RTL:
      tx -= delta;
      break;
    case GLCD_DIR_DOWN:
      ty += delta;
      break;
    case GLCD_DIR_UP:
      ty -= delta;
      break;
  }

  return 1;
}

int16_t
glcd_get_str_width_P(const char *s)
{
  int16_t w = 0;

  for (;;)
  {
    uint8_t encoding = pgm_read_byte(s++);
    if (encoding == 0)
      break;

    w += glcd_get_glyph_width(encoding);
  }

  return w;
}

/*
  -- Ethersex META --
  header(hardware/lcd/glcd/glcd.h)
  init(glcd_init)
*/
