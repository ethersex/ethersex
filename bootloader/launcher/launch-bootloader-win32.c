/****************************************************************************
* launch-bootloader implementation for win32                                *
* ------------------------------------------                                *
* (c) 2007 by Tim Mecking <tim@mecking.net>                                 *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
* General Public License for more details.                                  *
*                                                                           *
* A copy of the GNU General Public License can be obtained from the world   *
* wide web using the URL http://www.fsf.org/licensing/licenses/gpl.txt      *
*                                                                           *
* How to compile:                                                           *
* ---------------                                                           *
* Adjust path of include file in line 30 and compile with:                  *
* - Using OpenWatcom:   wcl386 launch-bootloader.c                          *
* - Using GCC:          gcc -o launch-bootloader.exe launch-bootloader.c    *
* - Using MS VisualC++: cl launch-bootloader.c                              *
****************************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "..\config.h"

#define APPNAME "launch-bootloader"
#define APPVER "Win32 1.0"
#define DELAY_TIME 50
#define WITH_SPINCHAR

const char cStrId[]="$Id$";

int main(int argc,char** argv) {
    const char cStrHelp[]="--help";
#ifdef WITH_SPINCHAR
    const char cStrSpin[]="-\\|/";
#endif
    int i;
    char c;
    DWORD dw;
    DWORD dwBaudRate;
    HANDLE hCom;
    DCB dcb;
    COMMTIMEOUTS commTimeouts;

    // Print usage info if parameter count is not 2 or --help/-h given
    if((argc!=3)||
            !stricmp(argv[1],cStrHelp)||!strnicmp(argv[1],&cStrHelp[1],2)||
            !stricmp(argv[2],cStrHelp)||!strnicmp(argv[2],&cStrHelp[1],2)) {
        fprintf(stderr,APPNAME " " APPVER "\n\
\n\
Continuesly sends bootloader entry char \"%c\" until the bootloader has been\n\
started and replied with the bootloader success char \"%c\".\n\
\n\
Usage: " APPNAME " <port> <bps>\n\
e.g. " APPNAME " COM1 9600\n",BOOTLOADER_ENTRY_CHAR,BOOTLOADER_SUCCESS_CHAR);
        return 1;
    }

    // Check if COM port is valid
    if(sscanf(argv[1],"COM%d%c",&i,&c)!=1) {
        fprintf(stderr,"Error: \"%s\" is not a com port name\n",argv[1]);
        return 1;
    }

    // Parse bitrate
    switch(atoi(argv[2])) {
        case 4800:
            dwBaudRate=CBR_4800;
            break;
        case 9600:
            dwBaudRate=CBR_9600;
            break;
        case 19200:
            dwBaudRate=CBR_19200;
            break;
        case 38400:
            dwBaudRate=CBR_38400;
            break;
        case 57600:
            dwBaudRate=CBR_57600;
            break;
        case 115200:
            dwBaudRate=CBR_115200;
            break;
        default:
            fprintf(stderr,"Error: \"%s\" is not a supported bitrate\n",argv[2]);
            return 1;
    }

	// Open COM port
    if((hCom=CreateFile(argv[1],
			GENERIC_READ | GENERIC_WRITE,
			0,    // must be opened with exclusive-access
			NULL, // no security attributes
			OPEN_EXISTING, // must use OPEN_EXISTING
			0,    // not overlapped I/O
			NULL// hTemplate must be NULL for comm devices
			))==INVALID_HANDLE_VALUE) {
        fprintf(stderr,"Error: failed to open port \"%s\"\n",argv[1]);
		return 1;
	}

	// Initialize COM port
    if(!GetCommState(hCom, &dcb)) {
		CloseHandle(hCom);
		fprintf(stderr,"Error: failed to initialize port \"%s\"\n",argv[1]);
		return 1;
	}
	dcb.BaudRate = dwBaudRate;     // set the baud rate
	dcb.ByteSize = 8;             // data size, xmit, and rcv
	dcb.Parity = NOPARITY;        // no parity bit
	dcb.StopBits = ONESTOPBIT;    // one stop bit
	dcb.fOutxCtsFlow = 0;
	dcb.fOutxDsrFlow = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fDsrSensitivity = 0;
	if(!SetCommState(hCom,&dcb)) {
		CloseHandle(hCom);
		fprintf(stderr,"Error: failed to initialize port \"%s\"\n",argv[1]);
		return 1;
	}

	// Set read/write operations to non-blocking mode
    commTimeouts.ReadIntervalTimeout=MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier=0;
	commTimeouts.ReadTotalTimeoutConstant=0;
    commTimeouts.WriteTotalTimeoutMultiplier=0;
	commTimeouts.WriteTotalTimeoutConstant=0;
	if(!SetCommTimeouts(hCom,&commTimeouts)) {
        CloseHandle(hCom);
        fprintf(stderr,"Error: failed to initialize port \"%s\"\n",argv[1]);
        return 1;
    }

    // Print info
    printf(APPNAME " " APPVER"\n\n1. Power off target device\n2. Connect target device to Port \"%s\"\n3. Power up target device\n ",argv[1]);


    for(i=0;;) {
#ifdef WITH_SPINCHAR
        printf("\b%c",cStrSpin[(i=(i+1)%(sizeof(cStrSpin)-1))]);
        fflush(stdout);
#endif
        // Clear input and output buffers
        PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);

        // Send Bootloader entry character
        c=BOOTLOADER_ENTRY_CHAR;
        if(WriteFile(hCom,&c,1,&dw,NULL)) {

            // Wait DELAY_TIME and check if bootloader replied with success character
            Sleep(DELAY_TIME);
            if((ReadFile(hCom,&c,1,&dw,NULL))&&(dw==1)&&(c==BOOTLOADER_SUCCESS_CHAR)) break;
            continue;
        }

        // Sleep
        Sleep(DELAY_TIME/2);
    }

    // Print info
    printf("\nBootloader launched\n");

    // Close COM port and exit with return code 0
    CloseHandle(hCom);
    return 0;
}
