#ifdef MCUF_MODUL_BORG16_INVADERS_SUPPORT
#include "invaders2.h"

void procCannon(Cannon * cn, uPixel * shot)
{
	static unsigned char mv = 0;
	if (mv >= CANNON_SPEED)
	{
		mv = 0;
		if (JOYISLEFT)
		{
			if (cn->pos != RIGHT_BORDER)
			{
				cn->pos++;
			}
		}
		else if (JOYISRIGHT)
		{
			if (cn->pos != LEFT_BORDER)
			{
				cn->pos--;
			}
		}
		else if (JOYISFIRE)
		{

			if (cn->ready)
			{
				shot->x = cn->pos;
				shot->y = 14;
				cn->ready = 0;
			}
		}

	}
	else
	{
		mv++;
	}

}

unsigned char areAtBorder(Invaders * iv)
{
	int y;
	for (y = SPACESHIP_LINE + 1; y <= GUARD_LINE; ++y)
	{
		if (getInvaderPixel(iv, LEFT_BORDER, y) || getInvaderPixel(iv,
				RIGHT_BORDER, y))
		{
			return 1;
		}
	}
	return 0;

}

void procInvaders(Invaders * iv, uPixel st[MAX_SHOTS])
{
	static unsigned char mv = 0;

	if (mv >= iv->speed)
	{
		mv = 0;
		if (areAtBorder(iv) && !(iv->isEdged))
		{
			iv->pos.y++;
			iv->direction = -iv->direction;
			iv->isEdged = 1;
		}
		else
		{
			iv->pos.x += iv->direction;
			iv->isEdged = 0;
		}

	}
	mv++;

	unsigned char i, y;
	unsigned char spos = random8() % 16;
	if (spos >= BORG_WIDTH)
		return;

	unsigned char shoot = random8();

	if (shoot < SHOOTING_RATE)
	{
		for (i = 0; i < MAX_SHOTS; ++i)
		{
			if (st[i].x > BORG_WIDTH || st[i].y > BORG_HEIGHT)
			{

				for (y = GUARD_LINE; y > SPACESHIP_LINE; --y)
				{
					if (getInvaderPixel(iv, spos, y) != 0)
					{

						st[i].x = spos;
						st[i].y = y + 1;
						return;
					}
				}
			}
		} //for SHOTS
	}

}

void procShots(Invaders * iv, Player * pl, Cannon * cn, Spaceship * sc,
		unsigned char guards[BORG_WIDTH], uPixel st[MAX_SHOTS], uPixel * shot)
{

	int i;
	static unsigned char cmv = 0, imv = 0;

	// shuß mit einen struct mit dem shuß!!

	if (cmv >= CANNON_SHOOTING_SPEED)
	{
		cmv = 0;
		if (!(cn->ready))
		{
			shot->y--;
		}
		if (shot->y > BORG_HEIGHT)
		{
			cn->ready = 1;
		}
	}

	if (imv >= INVADER_SHOOTING_SPEED)
	{
		imv = 0;

		for (i = 0; i < MAX_SHOTS; ++i)
		{
			if ( /*st[i].x < BORG_WIDTH && */st[i].y < BORG_HEIGHT)
			{
				st[i].y++;
			}
		}
	}

	cmv++;
	imv++;

	/****************************************************************/
	/*                    TESTE OB GETROFFEN                        */
	/****************************************************************/

	// USER CANNON

	unsigned char tmp;
	if (!(cn->ready))
	{
		for (i = 0; i < MAX_SHOTS; ++i)
		{
			if (shot->x == st[i].x && shot->y == st[i].y)
			{
				st[i].x = 255;
				st[i].y = 255;
				cn->ready = 1;
			}
		}

		//GUARDS
		if ((tmp = getGuardPixel(guards, shot->x, shot->y)))
		{
			--tmp;
			setGuardPixel(guards, shot->x, shot->y, tmp);
			cn->ready = 1;
			goto invader_shots;
		}

		//INVADER
		if ((tmp = getInvaderPixel(iv, shot->x, shot->y)))
		{
			--tmp;
			setInvaderPixel(iv, shot->x, shot->y, tmp);

			if (tmp == 0)
			{
				iv->speedinc++;
				if (iv->speedinc == SPEED_INC_RATE)
				{
					iv->speedinc = 0;
					iv->speed -= SPEED_INC_VALUE;
				}
				pl->points += POINTS_FOR_KILL;
			}
			else
			{
				pl->points += POINTS_FOR_HIT;
			}
			cn->ready = 1;
			goto invader_shots;
		}

		//SPACESHIP  

		if (shot->y == SPACESHIP_LINE)
		{
			if (shot->x == sc->pos || shot->x == sc->pos + 1)
			{
				sc->pos = 255;
				pl->points += POINTS_FOR_SPACESHIP;
				cn->ready = 1;
				goto invader_shots;
			}
		}
	} // !(cn->ready)


	invader_shots: for (i = 0; i < MAX_SHOTS; ++i)
	{
		if ((tmp = getGuardPixel(guards, st[i].x, st[i].y)))
		{
			--tmp;
			setGuardPixel(guards, st[i].x, st[i].y, tmp);
			st[i].x = 255;
			st[i].y = 255;
		}

		if (st[i].y == BORG_HEIGHT - 1)
		{
			if (st[i].x == cn->pos)
			{

				pl->lives--;
				st[i].x = 255;
				st[i].y = 255;
			}
		}
	}

}

void procSpaceship(Spaceship * sc)
{
	unsigned char rnd1 = random8();
	unsigned char rnd2 = random8();

	static unsigned char sct = 0;

	if (sc->pos > RIGHT_BORDER)
	{

		if (rnd1 == 73)
		{
			if (rnd2 >= 200)
			{
				sc->pos = RIGHT_BORDER;
				sct = 0;
			}
		}

	}
	else
	{
		if (sct == SPACESHIP_SPEED)
		{
			sct = 0;
			if (sc->pos == 0)
			{
				sc->pos = 255;
			}
			else
			{
				sc->pos--;
			}
		}
	}
	sct++;
}

unsigned char getStatus(Invaders * iv)
{

	//count Invader!
	unsigned char x, y, inv = 0;
	for (x = 0; x < MAX_INVADER_WIDTH; ++x)
	{
		for (y = 0; y < MAX_INVADER_HEIGHT; ++y)
		{
			if (iv->map[x][y] != 0)
				inv++;
		}
	}

	//LEVEL BEREINIGT
	if (inv == 0)
		return 1;

	//INVADERS REACHED EARTH
	for (x = 0; x < BORG_WIDTH; ++x)
	{
		if (getInvaderPixel(iv, x, GUARD_LINE + 1))
			return 2;
	}

	return 0;
}
#endif //MCUF_MODUL_BORG16_INVADERS_SUPPORT
