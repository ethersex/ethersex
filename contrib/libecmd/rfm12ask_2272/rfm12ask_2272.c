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

#include "libecmd.h"

/* error and die */
void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char *argv[])
{

	if (argc!=5 || (argc==2 && strcmp(argv[1],"--help")==0))
	{
		printf("Help for rfm12ask_2272 tool:\n"
		"\t[TARGET] [DIP_SWITCH_POSITIONS] [CHANNEL] [on/off]: set CHANNEL to on or off\n"
		"\tExample for adresscode 1-0-1-0-1, setting channel A to on:\n"
		"\t[TARGET] 10101 A on\n\n"
		"\t[TARGET] examples:\n"
		"\tusb://16c005dc for usb connection with vendor+product id\n"
		"\tudp://192.168.1.2:2701 for udp connection on ip 192.168.1.2 with port 2701\n"
		"\ttcp://192.168.1.2:2701 for tcp connection on ip 192.168.1.2 with port 2701\n"
		"\trs232:///dev/ttyS0:19200 for rs232 connection on device /dev/ttyS0 with baudrate 19200\n"
		);
		return 1;
	}
	if (strlen(argv[1])<5 || (strncmp(argv[1],"rs232",5)!=0 && strncmp(argv[1],"usb",3)!=0 && strncmp(argv[1],"udp",3)!=0 && strncmp(argv[1],"tcp",3)!=0))
	{
		printf("Wrong target format:\n"
		"\t[TARGET] examples:\n"
		"\tusb://16c005dc for usb connection with vendor+product id\n"
		"\tudp://192.168.1.2:2701 for udp connection on ip 192.168.1.2 with port 2701\n"
		"\ttcp://192.168.1.2:2701 for tcp connection on ip 192.168.1.2 with port 2701\n"
		"\trs232:///dev/ttyS0:19200 for rs232 connection on device /dev/ttyS0 with baudrate 19200\n"
		);
		return 1;
	}
	if (strlen(argv[2]) != 5)
	{
		printf("Wrong command format:\n"
		"\tYou have 5 DIP switches on your remote. Please enter 0 for off and 1 for on.\n"
		"\tExample for a valid adresscode: 10101\n");
		return 1;
	}

	if (strlen(argv[3]) != 1 || argv[3][0]<'A' || argv[3][0]>'E')
	{
		printf("Wrong command format:\n"
		"\tOnly channels between A-E are valid.\n");
		return 1;
	}

	// init buffer
	char ecmd[30];
	memset(ecmd, 0, sizeof(ecmd));

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
	uint8_t buffer[3];
	memset(buffer, 0, sizeof(buffer));
	char chan = argv[3][0];

	// Calculate first command byte
	if (argv[2][0]=='0') buffer[0] |= _BV(6);
	if (argv[2][1]=='0') buffer[0] |= _BV(4);
	if (argv[2][2]=='0') buffer[0] |= _BV(2);
	if (argv[2][3]=='0') buffer[0] |= _BV(0);
	// Calculate second command byte
	if (argv[2][4]=='0') buffer[1] |= _BV(6);
	if (chan=='B'||chan=='C'||chan=='D'||chan=='E')
		buffer[1] |= _BV(4);
	if (chan=='A'||chan=='C'||chan=='D'	||chan=='E')
		buffer[1] |= _BV(2);
	if (chan=='A'||chan=='B'||chan=='D'||chan=='E')
		buffer[1] |= _BV(0);
	// Calculate third command byte
	if (chan=='A'||chan=='B'||chan=='C'||chan=='E')
		buffer[2] |= _BV(6);
	if (chan=='A'||chan=='B'||chan=='C'||chan=='D')
		buffer[2] |= _BV(4);
	if (argv[4][0]=='1'||strcmp(argv[4],"on")==0) {
		buffer[2] |= _BV(0);
	} else {
		buffer[2] |= _BV(2);
	}

	sprintf(ecmd,"rfm12 2272 %d,%d,%d\n",buffer[0],buffer[1],buffer[2]);

	ecmd_connection* c = new ecmd_connection();
	if (c->add(argv[1])==0)
	{
		std::cerr << "Fatal: device adding failed" << std::endl;
		delete c;
		return 1;
	}

	std::cout << "Execute: " << ecmd << "Answer: " << c->execute(ecmd, 1500) << std::endl;
	delete c;
	return 0;
}
