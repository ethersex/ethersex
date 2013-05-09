/*
 * Copyright (c) 2009 by David Gräff <david.graeff@web.de>
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

#ifndef STELLA_FADING_FUNCTIONS_H
#define STELLA_FADING_FUNCTIONS_H

static void
stella_fade_normal(int i)
{
  if (stella_brightness[i] > stella_fade[i])
    stella_brightness[i]--;
  else                          /* stella_color[i] < stella_fade[i] */
    stella_brightness[i]++;
}


static void
stella_fade_flashy(int i)
{
  /* local copies, access>=3 times */
  uint8_t current = stella_brightness[i];
  uint8_t target = stella_fade[i];

  if (current > target)
    target <<= 1;
  if (current < target)
    current = target;

  stella_brightness[i] = current;
}


struct fadefunc_struct
{
  void (*p) (int i);
};

static struct fadefunc_struct stella_fade_funcs[FADE_FUNC_LEN] = {
  {stella_fade_normal},
  {stella_fade_flashy},
};

#endif /* STELLA_FADING_FUNCTIONS_H */
