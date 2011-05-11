/*
 *
 * Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
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
#include "config.h"
#ifdef STARBURST_SUPPORT

#ifdef STARBURST_PCA9685_EXTDRV
	#define STARBURST_PCA9685_EXTDRV 1
#else
	#define STARBURST_PCA9685_EXTDRV 0
#endif
#ifdef STARBURST_PCA9685_IVRT
	#define STARBURST_PCA9685_IVRT 1
#else
	#define STARBURST_PCA9685_IVRT 0
#endif
/*Defines for starburst_channel element "update"*/
#define STARBURST_UPDATE 1
#define STARBURST_NOUPDATE 0

struct starburst_channel {
	uint8_t value;
	uint8_t update;
};
void starburst_init();
void starburst_process();
#endif
