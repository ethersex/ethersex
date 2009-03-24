/*
Author: David Gräff <david.graeff@web.de>
Date: Feb, 2009
Purpose: Test stella routines of the ethersex project. (www.ethersex.de)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 or version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

////////////////////////////////////////////
// Enter here your server ip and udp port //
#define STELLA_PORT 2342
#define STELLA_IP "192.168.1.10"
////////////////////////////////////////////

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../stella/stella.h"

/* error and die */
void diep(char *s)
{
	perror(s);
	exit(1);
}

void check(unsigned int channel, unsigned int value)
{
	if (channel>255)
	{
		printf("CHANNEL_NUMBER must be between 0 and 255!\n");
		exit(1);
	}
	if (value>255)
	{
		printf("VALUE must be between 0 and 255!\n");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	if (argc==2 && strcmp(argv[1],"--help")==0)
	{
		printf("Help for stella test tool:\n"
		"\tEnter nothing to set all channels to random values\n"
		"\tset [CHANNEL_NUMBER] [VALUE]: set to VALUE (0-255)\n"
		"\tfade [CHANNEL_NUMBER] [VALUE]: fade to VALUE (0-255)\n"
		"\tflash [CHANNEL_NUMBER] [VALUE]: flash to 255 brightness and fade down to VALUE\n"
		"\tfadestep [STEPVALUE]: Set the fadestep to STEPVALUE, 1 is minimum\n");
		return;
	}

	/* initialize random seed: */
	srand ( time(NULL) );

	// init buffer
	char buf[16];
	int buflen;
	buflen = 0;

	// fill buffer with data
	if (argc==3 && strcmp(argv[1],"fadestep")==0)
	{ //fadestep
		unsigned int fadestep = atoi (argv[2]);
		if (fadestep>255)
		{
			printf("STEPVALUE must be between 0 and 255!\n");
			exit(1);
		}
		buflen = 2;
		buf[0] = STELLA_FADE_STEP;
		buf[1] = (char)fadestep;
	}
	else if (argc==4 && strcmp(argv[1],"set")==0)
	{ //fadestep
		unsigned int channel = atoi (argv[2]);
		unsigned int value = atoi (argv[3]);
		check(channel, value);
		buflen = 2;
		buf[0] = STELLA_SET_COLOR_0+channel;
		buf[1] = (char)value;
	}
	else if (argc==4 && strcmp(argv[1],"fade")==0)
	{ //fade
		unsigned int channel = atoi (argv[2]);
		unsigned int value = atoi (argv[3]);
		check(channel, value);
		buflen = 2;
		buf[0] = STELLA_FADE_COLOR_0+channel;
		buf[1] = (char)value;
	}
	else if (argc==4 && strcmp(argv[1],"flash")==0)
	{ //flash
		unsigned int channel = atoi (argv[2]);
		unsigned int value = atoi (argv[3]);
		check(channel, value);
		buflen = 2;
		buf[0] = STELLA_FLASH_COLOR_0+channel;
		buf[1] = (char)value;
	}
	else if (argc==1)
	{ // random data
		int i;
		for (i=0; i<8;++i)
		{
			buf[i*2] = (unsigned char)i;
			buf[i*2+1] = rand() % 255;
		}
		buflen = 16;
	}
	else
	{
		printf("Unknown command. Use --help\n");
		exit(1);
	}

	// create an udp socket
	struct sockaddr_in remote;
	int fd, i, slen=sizeof(remote);

	if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(STELLA_PORT);
	remote.sin_addr.s_addr = inet_addr(STELLA_IP);

	// connect to the socket
	if (connect(fd, (struct sockaddr*)&remote, sizeof(remote)) < 0) diep("connect");
	printf("Socket connection established!\n");

	// send
	printf("Sending %d bytes to the stella server %s and udp port %d!\n", buflen, STELLA_IP, STELLA_PORT);
	if ( write(fd, buf, buflen) == -1) { close(fd); diep("write"); }
	close(fd);

	return 0;
}
