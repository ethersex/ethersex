/*
Author: David Gräff <david.graeff@web.de>
Date: Feb, 2009
Purpose: Test ecmds routines of the ethersex project. (www.ethersex.de)

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
#define ECMDS_PORT 2701
#define ECMDS_IP "192.168.1.10"
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
#include "../../../protocols/ecmd/speed_parser.h"

#define _BV(bit) (1 << (bit))

/* error and die */
void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char *argv[])
{
	//FIXME: Currently only 2272

	if (argc!=4 || (argc==2 && strcmp(argv[1],"--help")==0))
	{
		printf("Help for rfm12ask_send tool (only 2272 currently):\n"
		"\t[DIP_SWITCH_POSITIONS] [CHANNEL] [on/off]: set CHANNEL to on or off\n"
		"\tExample for adresscode 1-0-1-0-1, setting channel A to on:\n"
		"\t10101 A on\n");
		return;
	}
	if (strlen(argv[1]) != 5)
	{
		printf("Wrong command format:\n"
		"\tYou have 5 DIP switches on your remote. Please enter 0 for off and 1 for on.\n"
		"\tExample for a valid adresscode: 10101\n");
		return;
	}

	if (strlen(argv[2]) != 1 || argv[2][0]<'A' || argv[2][0]>'E')
	{
		printf("Wrong command format:\n"
		"\tOnly channels between A-E are valid.\n");
		return;
	}

	// init buffer
	struct speed_parser_packet
	{
		uint8_t initcmd;
		uint8_t cmd;
		uint8_t data[7];
	} buf;
	buf.initcmd = '\n';
	buf.cmd = ECMDS_SET_RFM12ASK_SEND;
	memset(buf.data, 0, 7);

	char chan = argv[2][0];

	/*
	Tasten  |  Code
	--------+-----------------
	A    | 00 01 01 01 01
	B    | 01 00 01 01 01
	C    | 01 01 00 01 01
	D    | 01 01 01 00 01
	E    | 01 01 01 01 00
	*/

	/* Adresscodes are inverted, so check 0 */

	// Calculate first command byte
	if (argv[1][0]=='0') buf.data[0] |= _BV(6);
	if (argv[1][1]=='0') buf.data[0] |= _BV(4);
	if (argv[1][2]=='0') buf.data[0] |= _BV(2);
	if (argv[1][3]=='0') buf.data[0] |= _BV(0);
	// Calculate second command byte
	if (argv[1][4]=='0') buf.data[1] |= _BV(6);
	if (chan=='B'||chan=='C'||chan=='D'||chan=='E')
		buf.data[1] |= _BV(4);
	if (chan=='A'||chan=='C'||chan=='D'||chan=='E')
		buf.data[1] |= _BV(2);
	if (chan=='A'||chan=='B'||chan=='D'||chan=='E')
		buf.data[1] |= _BV(0);
	// Calculate third command byte
	if (chan=='A'||chan=='B'||chan=='C'||chan=='E')
		buf.data[2] |= _BV(6);
	if (chan=='A'||chan=='B'||chan=='C'||chan=='D')
		buf.data[2] |= _BV(4);
	if (argv[3][0]=='1'||strcmp(argv[3],"on")==0) {
		buf.data[2] |= _BV(0);
	} else {
		buf.data[2] |= _BV(2);
	}
	// set delay byte
	buf.data[3] = 72;
	// set repeat byte
	buf.data[4] = 4;


	int j;
	printf("Send command data: ");
	for (j=0;j<3;++j)
		printf("%i ", (int)(buf.data[j]));
	printf("\n");


	// create an udp socket
	struct sockaddr_in remote;
	int fd, i, slen=sizeof(remote);

	if ((fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	memset((char *) &remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_port = htons(ECMDS_PORT);
	remote.sin_addr.s_addr = inet_addr(ECMDS_IP);

	// connect to the socket
	if (connect(fd, (struct sockaddr*)&remote, sizeof(remote)) < 0) diep("connect");
	printf("Socket connection established!\n");

	// send
	printf("Sending %d bytes to the stella server %s and udp port %d!\n", sizeof(buf), ECMDS_IP, ECMDS_PORT);
	if ( write(fd, (void*)&buf, sizeof(buf)) == -1) { close(fd); diep("write"); }
	close(fd);

	return 0;
}
