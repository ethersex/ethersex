/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/wdt.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../global.h"
#include "tetrirape.h"

/*
 * overall tetrirape state
 */

/* list of connected players, and state of each */
static struct tetrirape_conn {
    unsigned       used           :1;
    unsigned       lost           :1;
    unsigned       team           :4;
    unsigned char  nickname[16];
} conns[6];

static struct {
    unsigned       playing        :4;
    unsigned       active         :6;		/* bitfield of taken conns */
} state;

#define QUEUE_BUF_LEN              35
#define QUEUE_LEN                  40
static struct queue_slot_t {
    uint8_t        recipients;
    uint8_t        ack_wait;

    uint8_t        length;
    unsigned char  data[QUEUE_BUF_LEN];
} queue[QUEUE_LEN];
static uint8_t queue_head;			/* what to send next */


/*
 * cracker prototypes
 */
extern void tet_dec2str(char *buf);
extern int tet_decrypt(char *buf);


/*
 * macros
 */
#define playernum()  (uip_conn->appstate.tetrirape.player)
#define playerconn() (conns[uip_conn->appstate.tetrirape.player])

#define tetrirape_queue_all_except(r,b,i) \
    (_tetrirape_queue(state.active & ~(1 << (r)), b, i))
#define tetrirape_queue_all(b,i) \
    (_tetrirape_queue(state.active, b, i))
#define tetrirape_queue(b,i) \
    (_tetrirape_queue((1 << playernum()), b, i))



static void
freakout(void)
{
    DDRC |= _BV(2);
    PORTC |= _BV(PC2);				/* enable led */
    for(;;)  					/* rest in peace */
	wdt_reset();
}



#define rel_q(i) (queue[(i + queue_head) % QUEUE_LEN])


/*
 * several routines which try to make more queue slots available
 * 
 *  -> just rearrange slots, i.e. arrange packages immediately one
 *     after another
 *  -> clear ack_waits (retransmits won't work afterwards)
 */
static void
tetrirape_queue_clean(uint8_t level)
{
    if(level >= 3) 
	freakout();


    int i;
    if(level >= 2)				/* clear acks */
	for(i = 0; i < QUEUE_LEN; i ++)
	    queue[i].ack_wait = 0;

    
    if(level >= 1) {			        /* rearrange used slots */
	uint8_t src = 0, dest = 0;
	for(; src < QUEUE_LEN; src ++) {
	    if(! rel_q(src).recipients && ! rel_q(src).ack_wait)
		continue;			/* empty slot */
	    
	    for(; dest < src; dest ++)
		if(! rel_q(dest).recipients && ! rel_q(dest).ack_wait)
		    break;
	
	    if(src == dest) {
		dest ++;
		continue;			/* nothing to move */
	    }

	    /* copy data over, then clear src */
	    memcpy(&rel_q(dest), &rel_q(src), sizeof(struct queue_slot_t));
	    rel_q(src).recipients = 0;
	    rel_q(src).ack_wait = 0;
	    
	    dest ++;
	}
    }
}


/*
 * queue a packet that shall be sent later on ...
 */
static void
_tetrirape_queue(uint8_t rcpt, const unsigned char *buf, uint16_t len)
{
    int level = 0;
    while(len) {
	tetrirape_queue_clean(level ++);

	int8_t not_before;
	for(not_before = QUEUE_LEN - 1; not_before >= 0; not_before --)
	    if(rel_q(not_before).recipients & rcpt)
		break;				/* cannot queue before here */

	if(not_before == QUEUE_LEN - 1)
	    continue;				/* queue is full */

	int8_t i = not_before + 1;
	
	while(len)
	    for(; i < QUEUE_LEN; i ++)
		if(! rel_q(i).recipients) {
		    rel_q(i).recipients = rcpt;
		    rel_q(i).length =
			(len > QUEUE_BUF_LEN) ? QUEUE_BUF_LEN : len;
		    memcpy(rel_q(i).data, buf, rel_q(i).length);

		    buf += rel_q(i).length;
		    len -= rel_q(i).length;
		    break;
		}
    }
}



void
tetrirape_try_unqueue(int mode)
{
    uint8_t mask = 1 << playernum();
    uint8_t i;
    uint16_t l = 0;

    /* put all packets that shall be sent into uip_appdata */
    for(i = 0; i < QUEUE_LEN; i ++)
	if((mode ? rel_q(i).ack_wait : rel_q(i).recipients) & mask) {
	    if(mode != 2) {
		if(l + rel_q(i).length > uip_mss())
		    break;			/* mss exceeded */

		memcpy(uip_appdata + l, rel_q(i).data, rel_q(i).length);
		l += rel_q(i).length;
		rel_q(i).ack_wait |= mask;
	    }
	    else
		rel_q(i).ack_wait &= ~mask;

	    if(mode == 0)
		rel_q(i).recipients &= ~mask;
	}

    if(l)
	uip_send(uip_appdata, l);

    /* move queue_head forward */
    for(i = 0; i < QUEUE_LEN; i ++)
	if(rel_q(i).recipients || rel_q(i).ack_wait) {
	    queue_head = (queue_head + i) % QUEUE_LEN;
	    break;
	}
}


static void
tetrirape_eat_cmd_team(void)
{
    /* we don't support teams (yet), therefore we've got to:
     *
     *   a) inform others that the particular player has joined
     *   b) the particular player who else is there
     */

    int i;

    /* tell others of this client ... */
    i = sprintf(uip_appdata, "playerjoin %d %s\xff",
		playernum() + 1, playerconn().nickname);
    tetrirape_queue_all_except(playernum(), uip_appdata, i);

    /* tell new player of all the others ... */
    int j; for(j = 0; j < 6; j ++) {
	if(! conns[j].used)
	    continue;

	i = sprintf(uip_appdata, "playerjoin %d %s\xFF",
		    j + 1, conns[j].nickname);
	tetrirape_queue(uip_appdata, i);
    }

    i = sprintf(uip_appdata, "pline 0 %s: pleased to meet you!\xFF", 
		playerconn().nickname);
    tetrirape_queue(uip_appdata, i);
    const unsigned char *welcome_msg_2 = "pline 0 this is tetrirape.\xFF";
    tetrirape_queue(welcome_msg_2, 27);
}


static void
tetrirape_eat_cmd_startgame(void)
{
    char *ptr = uip_appdata;

    if(ptr[10] == '0' && state.playing) {
	const unsigned char *endgame_msg = "endgame\xff";
	tetrirape_queue_all(endgame_msg, 8);
	state.playing = 0;
	return;
    }

    /* start the new game */
    uint8_t specials[9] = CONFIG_SPECIALS;
    uint8_t pieces[7] = CONFIG_PIECES;
    uint8_t x, total = 0;
    
    uint8_t n = sprintf(ptr, "newgame 0 %d %d %d %d %d %d ",
			CONFIG_INITIAL_LEVEL, 
			CONFIG_LINES_PER_LEVEL,
			CONFIG_LEVEL_INC,
			CONFIG_SPECIAL_LINES,
			CONFIG_SPECIAL_COUNT,
			CONFIG_SPECIAL_CAPACITY);
    ptr += n;

    /* pieces */
    for (x = 0; x < 7; x ++) {
	if(pieces[x])
	    memset(ptr, '1' + x, pieces[x]);

	total += pieces[x];
	ptr += pieces[x];
    }

    if(total != 100)
	goto config_error;

    *(ptr ++) = ' ';

    /* specials */
    for(total = 0, x = 0; x < 9; x ++) {
	if(specials[x])
	    memset(ptr, '1' + x, specials[x]);

	total += specials[x];
	ptr += specials[x];
    }

    if(total != 100) 
	goto config_error;

    *(ptr ++) = ' ';

    n = sprintf(ptr, "%d %d\xff", 
		CONFIG_LEVEL_AVERAGE,
		CONFIG_CLASSIC_MODE);

    n += ((void *) ptr) - ((void *) uip_appdata);
    tetrirape_queue_all(uip_appdata, n);

    for(state.playing = 0, n = 0; n < 6; n ++)
	if(conns[n].lost = 0, conns[n].used)
	    state.playing ++;

    return;

config_error:
    tetrirape_queue_all("pline 0 configuration error.\xFF", 29);
    return;
}



static void
tetrirape_check_playerwon(void)
{
    if(state.playing > 1)
	return;

    tetrirape_queue_all("endgame\xFF", 8);

    uint8_t i;
    for(i = 0; i < 6; i ++)
	if(! conns[i].lost) {
	    sprintf(uip_appdata, "playerwon %d\xFF", i + 1);
	    tetrirape_queue_all(uip_appdata, 12);
	    int l = sprintf(uip_appdata, "pline 0 ---- Player %s\x04 WON "
	                    "----\xFF", conns[i].nickname);
	    tetrirape_queue_all(uip_appdata, l);
	    break;
	}

    state.playing = 0;
}

void
tetrirape_eat_data(void)
{
    /* we currently don't support one command spanning over several tcp packets
     *
     * unfortunately tetrinet-client (i.e. the console client) tends to send
     * the command in one packet and the \xFF in a separate one afterwards.
     *
     * we could cache the data, however we'd have to carry around states
     * and waste memory, therefore let's do a dirty workaround: 
     * let's terminate all commands no matter what :-)
     *
     * there are some commands that smash uip_appdata, among those are:
     *  - team
     *  - startgame
     *  - everything that results in a playerwon-check
     */
    if(((unsigned char *) uip_appdata)[uip_datalen() - 1] != 0xFF) {
	((unsigned char *) uip_appdata)[uip_datalen()] = 0xFF;
	uip_datalen() ++;
    }

    if(((unsigned char *) uip_appdata)[0] == 0xFF) {
	uip_appdata ++;
	uip_datalen() --;
    }
    
    if(! uip_datalen())
	return;
    
    void *ptr;
    uint16_t leftlen = uip_datalen();
    while(leftlen
	  && (ptr = strchr(uip_appdata, 0xFF),
	      ptr != uip_appdata + leftlen - 1)) {
	/* detected several commands in one tcp packet, let's split it up 
	 * care for first piece */
	uip_datalen() = (ptr - uip_appdata) + 1;
	tetrirape_eat_data();

	leftlen -= (ptr - uip_appdata) + 1;
	uip_appdata += (ptr - uip_appdata) + 1;
    }

    /* try to authenticate guest, if not done already ... */
    if(! uip_conn->appstate.tetrirape.authenticated) {
	/* new connection, do the strange decryption thingy */
	if(tet_decrypt(uip_appdata)) {
	    uip_close();
	    return;				/* decode failed */
	}

	tet_dec2str(uip_appdata);

	/* nickname is in `uip_appdata' now, try to queue ... */
	int i; for(i = 0; i < 6; i ++) {
	    if(conns[i].used)
		continue;

	    conns[i].used = 1;
	    conns[i].team = i;
	    conns[i].lost = 1;
	    memcpy(conns[i].nickname, uip_appdata, 16);
	    break;
	}
	
	if(i == 6) {
	    uip_close();
	    return;				/* no more slots left */
	}

	uip_conn->appstate.tetrirape.authenticated = 1;
	uip_conn->appstate.tetrirape.player = i;
	state.active |= 1 << i;
	
	/* greet our guest ... */
	char buf[21]; snprintf(buf, 21, "playernum %d\xffwinlist\xff", i + 1);
	tetrirape_queue(buf, 20);

	if(state.playing) {
	    const char *buf_ingame = "ingame\xff";
	    tetrirape_queue(buf_ingame, 7);
	}
    }

    else if(! strncmp(uip_appdata, "team", 4))
	tetrirape_eat_cmd_team();

    else if(! strncmp(uip_appdata, "pline", 5))
	tetrirape_queue_all_except(playernum(), uip_appdata, uip_datalen());

    else if(! strncmp(uip_appdata, "f ", 2))
	tetrirape_queue_all_except(playernum(), uip_appdata, uip_datalen());

    else if(! strncmp(uip_appdata, "startgame", 9))
	tetrirape_eat_cmd_startgame();

    else if(! strncmp(uip_appdata, "playerlost", 10)) {
	tetrirape_queue_all_except(playernum(), uip_appdata, uip_datalen());
	playerconn().lost = 1;
	if(state.playing) {
	    state.playing --;
	    tetrirape_check_playerwon();
	}
    }

    else if(! strncmp(uip_appdata, "sb", 2))
	tetrirape_queue_all_except(playernum(), uip_appdata, uip_datalen());

    else if(! strncmp(uip_appdata, "lvl", 3))
	tetrirape_queue_all_except(playernum(), uip_appdata, uip_datalen());

    else if(! strncmp(uip_appdata, "pause", 5))
	tetrirape_queue_all(uip_appdata, uip_datalen());

    else if(! strncmp(uip_appdata, "gmsg", 4))
	tetrirape_queue_all(uip_appdata, uip_datalen());
}


void
tetrirape_disconnect(void)
{
    /* tell others about leaving player */
    sprintf(uip_appdata, "playerleave %d\xFF", playernum() + 1);
    tetrirape_queue_all_except(playernum(), uip_appdata, 14);

    if(! playerconn().lost && state.playing) {
        /* player hasn't lost already ... */
        playerconn().lost = 1;
        state.playing --;

        tetrirape_check_playerwon();
    }

    state.active &= ~(1 << playernum());
    playerconn().used = 0;

    int i; for(i = 0; i < QUEUE_LEN; i ++) {
        queue[i].recipients &= state.active;
        queue[i].ack_wait &= state.active;
    }
}
