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

#include <stdio.h>
#include "libecmd.h"


/* error and die */
void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char *argv[])
{
	if (argc<2 || (argc==2 && strcmp(argv[1],"--help")==0))
	{
		printf("Help for interactive tool:\n"
		"\t[TARGET] [ecmd] - ecmd is optional, if given program does not enter interactive mode but just execute that ecmd\n"
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
	char ecmd[100];
	int ecmd_len;

	struct connection* c = ecmd_init();
	if (ecmd_add(c, argv[1])==0)
	{
		printf("Fatal: device adding failed\n");
		ecmd_close(c);
		return 0;
	}

	// only one command
	if (argc==3) {
		sprintf(ecmd, "%s\n", argv[1]);
		ecmd_len = strlen(ecmd);

		printf("Execute: ");
		printf("%s", ecmd);
		printf("Answer: ");
		printf("%s", ecmd_execute(c, ecmd, ecmd_len, 500));
	} else {
		printf("Interactive mode. Enter nothing and hit [Return] to leave.\n");
		char *line = 0;
		unsigned int len;
		while (getline(&line, &len, stdin) != -1)
		{
			if (strlen(line)==1) break;
			line[strlen(line) - 1] = 0;

			sprintf(ecmd, "%s\n", line);
			ecmd_len = strlen(ecmd);
			printf("###\n%s###\n", ecmd_execute(c, ecmd, ecmd_len, 500));
		}
	}
	ecmd_close(c);
	return 0;
}
