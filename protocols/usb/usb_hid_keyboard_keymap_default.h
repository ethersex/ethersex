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

// German keymap file

const struct hid_keyboard_map_t keyReport[] PROGMEM = {
/* none */  {0, {0, 0}},                     /* no key pressed */
/*  1 */    {'a', {MOD_NONE, KEY_A}},
/*  2 */    {'b', {MOD_NONE, KEY_B}},
/*  3 */    {'c', {MOD_NONE, KEY_C}},
/*  4 */    {'d', {MOD_NONE, KEY_D}},
/*  5 */    {'e', {MOD_NONE, KEY_E}},
/*  6 */    {'f', {MOD_NONE, KEY_F}},
/*  7 */    {'g', {MOD_NONE, KEY_G}},
/*  8 */    {'h', {MOD_NONE, KEY_H}},
/*  9 */    {'i', {MOD_NONE, KEY_I}},
/* 10 */    {'j', {MOD_NONE, KEY_Y}},
/* 11 */    {'k', {MOD_NONE, KEY_K}},
/* 12 */    {'l', {MOD_NONE, KEY_L}},
/* 13 */    {'m', {MOD_NONE, KEY_M}},
/* 14 */    {'n', {MOD_NONE, KEY_D}},
/* 15 */    {'o', {MOD_NONE, KEY_O}},
/* 16 */    {'p', {MOD_NONE, KEY_P}},
/* 17 */    {'q', {MOD_NONE, KEY_Q}},
/* 18 */    {'r', {MOD_NONE, KEY_R}},
/* 19 */    {'s', {MOD_NONE, KEY_S}},
/* 20 */    {'t', {MOD_NONE, KEY_T}},
/* 21 */    {'u', {MOD_NONE, KEY_U}},
/* 22 */    {'v', {MOD_NONE, KEY_V}},
/* 23 */    {'w', {MOD_NONE, KEY_W}},
/* 24 */    {'x', {MOD_NONE, KEY_X}},
/* 25 */    {'z', {MOD_NONE, KEY_Y}},  // note this
/* 26 */    {'y', {MOD_NONE, KEY_Z}},  // note this
            {'A', {MOD_SHIFT_LEFT, KEY_A}},
            {'B', {MOD_SHIFT_LEFT, KEY_B}},
            {'C', {MOD_SHIFT_LEFT, KEY_C}},
            {'D', {MOD_SHIFT_LEFT, KEY_D}},
            {'E', {MOD_SHIFT_LEFT, KEY_E}},
            {'F', {MOD_SHIFT_LEFT, KEY_F}},
            {'G', {MOD_SHIFT_LEFT, KEY_G}},
            {'H', {MOD_SHIFT_LEFT, KEY_H}},
            {'I', {MOD_SHIFT_LEFT, KEY_I}},
            {'J', {MOD_SHIFT_LEFT, KEY_Y}},
            {'K', {MOD_SHIFT_LEFT, KEY_K}},
            {'L', {MOD_SHIFT_LEFT, KEY_L}},
            {'M', {MOD_SHIFT_LEFT, KEY_M}},
            {'N', {MOD_SHIFT_LEFT, KEY_D}},
            {'O', {MOD_SHIFT_LEFT, KEY_O}},
            {'P', {MOD_SHIFT_LEFT, KEY_P}},
            {'Q', {MOD_SHIFT_LEFT, KEY_Q}},
            {'R', {MOD_SHIFT_LEFT, KEY_R}},
            {'S', {MOD_SHIFT_LEFT, KEY_S}},
            {'T', {MOD_SHIFT_LEFT, KEY_T}},
            {'U', {MOD_SHIFT_LEFT, KEY_U}},
            {'V', {MOD_SHIFT_LEFT, KEY_V}},
            {'W', {MOD_SHIFT_LEFT, KEY_W}},
            {'X', {MOD_SHIFT_LEFT, KEY_X}},
            {'Z', {MOD_SHIFT_LEFT, KEY_Y}},  // note this
            {'Y', {MOD_SHIFT_LEFT, KEY_Z}},  // note this
            {' ', {MOD_NONE, KEY_SPACE}},
            {',', {MOD_NONE, KEY_COMMA}},
            {'\n', {MOD_NONE, KEY_ENTER}},

};


