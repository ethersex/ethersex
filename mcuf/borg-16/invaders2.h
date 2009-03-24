/*  	Space INVADERS V0.2 
 *		
 *		by: Fabian Bremerich
 *		Thanx to: Peter F. 
 *
 *		date: Mi, 08.03.2006 
 *
 */

#ifndef INVADERS2_H
#define INVADERS2_H

#include "../mcuf_modul.h"

#ifdef MCUF_MODUL_BORG16_INVADERS_SUPPORT

#define USE_ORIGINAL_PIXEL_API

/*CONNECTION TO SIMULATOR*/
//extern char fkey;

/* TEST PARTS NEW API */

typedef struct
{
	signed char x;
	signed char y;
} sPixel;

typedef struct
{
	unsigned char x;
	unsigned char y;
} uPixel;

//for compatibility to pisel.h api!
#ifdef USE_ORIGINAL_PIXEL_API
//#include "pixel.h"
//#include "scrolltext.h"
//#include "joystick.h"

//typedef uPixel pixel;
#define uPixel pixel
//#define getPixel(_X, _Y) get_pixel( (pixel){_X, _Y})
#define clearScreen() 	clear_screen(0)
//#define 

//#ifdef SIMULATOR
//#define setPixel(_X, _Y, _V) setpixel( (pixel){_X, _Y}, _V)
//#else //if defined (AVR)
//#define setPixel(_X, _Y, _V) reverseSetPixel( (pixel){_X, _Y}, _V)
//#endif

#endif

/****************************************************************/
/*                   GLOBALE VAR                                */
/****************************************************************/
#define P 3
extern unsigned char peter[8][11];
extern unsigned char hans[8][11];

/****************************************************************/
/*                          DEFINES                             */
/****************************************************************/
#define START_LIVES		3

#define SPACESHIP_LINE  1
//#define SPACESHIP_TRIGGER_POINTS 250
//#define SPACESHIP_TRIGGER_RATE 333


#define GUARD_LINE		13

#define BORG_WIDTH 16
#define BORG_HEIGHT 16

#ifdef SWITCHED_SIDE
#define RIGHT_BORDER 0
#define LEFT_BORDER	(BORG_WIDTH -1 )
#else
#define RIGHT_BORDER (BORG_WIDTH -1 )
#define LEFT_BORDER	0
#endif

#define MAX_INVADER_HEIGHT	8
#define MAX_INVADER_WIDTH	12
#define MAX_INVADER_LIVES	3

#define POINTS_FOR_HIT 			5
#define POINTS_FOR_KILL 		25
#define POINTS_FOR_SPACESHIP 	75
#define POINTS_FOR_LEVEL		100

#define MAX_SHOTS 	7
#define MIN_SPEED   	70
#define SPEED_INC_RATE 	2
#define SPEED_INC_VALUE 3
#define MAX_LEVEL 5

#define SHOOTING_RATE 	6
#define INVADER_SHOOTING_SPEED 10
#define CANNON_SHOOTING_SPEED  4
#define SPACESHIP_SPEED	30

#define CANNON_SPEED 2

#define WAIT_MS 		15
//#define WAIT_MS               20


typedef struct
{
	unsigned char map[MAX_INVADER_WIDTH][MAX_INVADER_HEIGHT];
	sPixel pos;

	unsigned char speed;
	unsigned char speedinc;
	signed char direction;
	unsigned char isEdged;
} Invaders;

typedef struct
{
	unsigned char pos;
	unsigned char lives;
} Spaceship;

typedef struct
{
	unsigned char pos;
	unsigned char ready;
} Cannon;

//typedef struct {
//      unsigned char guards[numGards];
//}

typedef struct
{
	signed char lives;
	unsigned int points;
} Player;

/****************************************************************/
/*                          FUNCTIONS                           */
/****************************************************************/

void borg_invaders();
/*----------------------main_level_funcs-------------------------*/

void procSpaceship(Spaceship * sp);
void procCannon(Cannon * cn, uPixel * shot);

void procInvaders(Invaders * iv, uPixel st[MAX_SHOTS]);
void procShots(Invaders * iv, Player * pl, Cannon * cn, Spaceship * sc,
		unsigned char guards[BORG_WIDTH], uPixel st[MAX_SHOTS], uPixel * shot);

unsigned char getStatus(Invaders * iv);

/*----------------------Initialization---------------------------*/
void initGuards(unsigned char guards[BORG_WIDTH]);
void initInvaders(Invaders * iv, unsigned char lv);
//void initSpaceship(Spaceship* sc);
//void initPlayer(Player* pl);

/*----------------------getter/setter----------------------------*/

unsigned char getInvaderPixel(Invaders * iv, unsigned char x, unsigned char y);

void setInvaderPixel(Invaders * iv, unsigned char x, unsigned char y,
		unsigned char val);

unsigned char getGuardPixel(unsigned char guards[BORG_WIDTH], unsigned char x,
		unsigned char y);

void setGuardPixel(unsigned char guards[BORG_WIDTH], unsigned char x,
		unsigned char y, unsigned char val);

/*----------------------drawing Method---------------------------*/

void draw(Invaders * iv, Spaceship * sc, Player * pl, Cannon * cn,
		unsigned char guards[BORG_WIDTH], uPixel ishots[MAX_SHOTS],
		uPixel * shot);

#endif // MCUF_MODUL_BORG16_INVADERS_SUPPORT

#endif
