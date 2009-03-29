/*
* Copyright (c) 2009 by David Gräff <david.graeff@web.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
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

static void
stella_fade_normal (int i)
{
	if (stella_color[i] > stella_fade[i])
		stella_color[i] --;
	else /* stella_color[i] < stella_fade[i] */
		stella_color[i] ++;
}


static void
stella_fade_flashy (int i)
{
	if (stella_color[i] > stella_fade[i])
		stella_color[i] <<= 1;

	if (stella_color[i] < stella_fade[i])
		stella_color[i] = stella_fade[i];
}


static struct
{
	void (* p) (int i);
} stella_fade_funcs[FADE_FUNC_LEN] =
{
	{ stella_fade_normal },
	{ stella_fade_flashy },
};
