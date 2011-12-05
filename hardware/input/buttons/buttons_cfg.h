/*
 * Copyright (c) 2011 by Daniel Walter <fordprfkt@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef BUTTONS_CFG_H_
#define BUTTONS_CFG_H_

/*****************************************************************
 * To configure additional buttons:
 * (1) Add a line in the form of
 *     _x(<NameOfYourButton>),\
 *     to the #define below. Omit the ',' on the last line.
 *
 * (2) Add a line in the form of
 *     pin(<Name of your Button>, <Pin of your button>, INPUT)
 *     to the pinning configuration you use.
 *
 * (3) Thats it :-)
 *
 * See the following example:
 * #define BTN_CONFIG(_x) \
 * _x(BTN_UP),\
 * _x(BTN_DOWN),\
 * _x(BTN_LEFT),\
 * _x(BTN_RIGHT),\
 * _x(BTN_ENTER)
 *
 * In the pinning configuration:
 * pin(BTN_UP, PC2, INPUT)
 * pin(BTN_RIGHT, PC3, INPUT)
 * pin(BTN_DOWN, PC4, INPUT)
 * pin(BTN_LEFT, PC5, INPUT)
 * pin(BTN_ENTER, PD2, INPUT)
 *
 */

/* Define your buttons here: */
#define BTN_CONFIG(_x) \
 _x(BTN_UP),\
 _x(BTN_DOWN),\
 _x(BTN_LEFT),\
 _x(BTN_RIGHT),\
 _x(BTN_ENTER)

#endif /* BUTTONS_CFG_H_ */
