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

/* error and die */
void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
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

	// our test data
	char buf[] = {0,10,  1,10,  2,5,  3,100,  4,70,  5,70};
	int buflen = sizeof(buf);
	
	printf("Sending %d bytes to the stella udp port %d!\n", buflen, STELLA_PORT);
	if ( write(fd, buf, buflen) == -1) { close(fd); diep("write"); }

	printf("Finished!\n");
	close(fd);
	return 0;
}
