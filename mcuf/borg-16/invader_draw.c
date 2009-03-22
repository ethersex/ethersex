#ifdef MCUF_MODUL_BORG16_INVADERS_SUPPORT
#include "invaders2.h"
/*----------------------getter/setter----------------------------*/

unsigned char getInvaderPixel(Invaders * iv, unsigned char x, unsigned char y)
{
	if (((x - iv->pos.x) < MAX_INVADER_WIDTH) && ((x - iv->pos.x) >= 0) && ((y
			- iv->pos.y) < MAX_INVADER_HEIGHT) && ((y - iv->pos.y) >= 0))
	{
		return iv->map[x - iv->pos.x][y - iv->pos.y];
	}
	return 0;
}

void setInvaderPixel(Invaders * iv, unsigned char x, unsigned char y,
		unsigned char val)
{
	if (((x - iv->pos.x) < MAX_INVADER_WIDTH) && ((x - iv->pos.x) >= 0) && ((y
			- iv->pos.y) < MAX_INVADER_HEIGHT) && ((y - iv->pos.y) >= 0))
	{
		iv->map[x - iv->pos.x][y - iv->pos.y] = val;
	}
}

unsigned char getGuardPixel(unsigned char guards[BORG_WIDTH], unsigned char x,
		unsigned char y)
{
	if (x < BORG_WIDTH && y == GUARD_LINE)
		return guards[x];
	return 0;
}

void setGuardPixel(unsigned char guards[BORG_WIDTH], unsigned char x,
		unsigned char y, unsigned char val)
{
	if (x < BORG_WIDTH && y == GUARD_LINE && val <= 3)
		guards[x] = val;
}

/*----------------------drawing Method---------------------------*/

void draw(Invaders * iv, Spaceship * sc, Player * pl, Cannon * cn,
		unsigned char guards[BORG_WIDTH], uPixel st[MAX_SHOTS], uPixel * shot)
{
	clearScreen ();

	int x, y;

	/*---SPACESHIP---*/
	if (sc->pos < RIGHT_BORDER)
	{
		setPixel (sc->pos, SPACESHIP_LINE, sc->lives);
	}
	if (sc->pos - 1 < RIGHT_BORDER)
	{
		setPixel (sc->pos + 1, SPACESHIP_LINE, sc->lives);
	}

	/*---INVADERS--*/
	for (y = 0; y < MAX_INVADER_HEIGHT; y++)
	{
		for (x = 0; x < MAX_INVADER_WIDTH; x++)
		{
			//mal in oder  statement umwandeln ;-)
			if (iv->map[x][y] == 0)
				continue;
			if (x + iv->pos.x > RIGHT_BORDER)
				continue;
			if (x + iv->pos.x < 0)
				continue;

			setPixel (x + iv->pos.x, y + iv->pos.y, iv->map[x][y]);
		}
	}

	/*---GUARDS---*/
	for (x = 0; x < BORG_WIDTH; ++x)
	{
		if (guards[x] != 0)
		{
			setPixel (x, GUARD_LINE, guards[x]);
		}
	}

	/*---SHOTS--*/
	int i;
	for (i = 0; i < MAX_SHOTS; ++i)
	{
		if (st[i].x < BORG_WIDTH && st[i].y < BORG_HEIGHT)
		{
			setPixel (st[i].x, st[i].y, 3);
		}
	}

	/*draw player shot */
	if (!(cn->ready))
	{
		setPixel (shot->x, shot->y, 3);
	}

	/*-- CANNON --*/
	if (cn->pos >= LEFT_BORDER + 1)
	{
		setPixel (cn->pos - 1, 15, pl->lives);
	}
	if (cn->pos < BORG_WIDTH)
	{
		setPixel (cn->pos, 15, pl->lives);
		setPixel (cn->pos, 14, pl->lives);
	}
	if (cn->pos < RIGHT_BORDER)
	{
		setPixel (cn->pos + 1, 15, pl->lives);
	}

}
#endif // MCUF_MODUL_BORG16_INVADERS_SUPPORT
