/*
 * version.h - export build time and version info as a global symbol.
 *
 * Copyright (c) 2016 Michael Brakemeier <michael@brakemeier.de>
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

/**
 * Provide version info and build date and time for ethersex.
 */

#include <avr/pgmspace.h>

#include "autoconf.h"

/* global version defines */
#define VERSION_STRING          GIT_VERSION
#define VERSION_STRING_LONG     GIT_VERSION " built on " __DATE__ " " __TIME__
#define E6_VERSION_STRING_LONG  "ethersex " VERSION_STRING_LONG

/* one global version string in program space, saves flash */
extern const char PROGMEM pstr_E6_VERSION_STRING_LONG[];
