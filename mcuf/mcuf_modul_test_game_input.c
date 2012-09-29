/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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

#include "config.h"
#ifdef MCUF_TEST_GAME_INPUT
#include "mcuf_modul.h"
#include "mcuf_modul_test_game_input.h"

#include "core/debug.h"

uint8_t gix = MCUF_MAX_SCREEN_WIDTH / 2;
uint8_t giy = MCUF_MAX_SCREEN_HEIGHT / 2;
uint8_t gic = 1;

void
test_game_input_handler(buttons_ButtonsType button, uint8_t status)
{
  if (BUTTON_PRESS == status)
  {
    switch (button)
    {
      case BTN_FIRE:  gic++; break;
      case BTN_FIRE2: gic--; break;
      case BTN_LEFT:  if (gix > 0) gix--; break;
      case BTN_RIGHT: if (gix < MCUF_MAX_SCREEN_WIDTH) gix++; break;
      case BTN_UP:    if (giy > 0) giy--; break;
      case BTN_DOWN:  if (giy < MCUF_MAX_SCREEN_HEIGHT) giy++; break;
    }
    setPixel(gix, giy, gic);
  }
}

void
test_game_input(void)
{
  // clear screen
  for (int y=0; y < MCUF_MAX_SCREEN_WIDTH; y++)
    for (int x=0; x < MCUF_MAX_SCREEN_HEIGHT; x++)
       setPixel(x, y, 0);

  setPixel(gix, giy, gic);

  hook_buttons_input_register(test_game_input_handler);
  debug_printf("mcuf game input init done\n");
}
#endif //MCUF_TEST_GAME_INPUT

