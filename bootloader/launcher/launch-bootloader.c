/* Foodloader launch utility
 * Scott Torborg - storborg@mit.edu - August 2006
 * modified by Alexander Neumann <alexander@bumpern.de>
 *
 * This utility is intended to be run with the WAIT_FOR_CHAR_MODE option
 * of the foodloader bootloader. When foodloader is compiled and installed
 * with this option, run this program before supplying power to the target
 * device. This program will simply send a constant stream of the character
 * 'p' and detect when the target device has successfully entered programming
 * mode. Then avrdude should be run to program the device.
 *
 * The executable returns 0 when the bootloader is successfully entered, so
 * it may be used in a Makefile.
 */

#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"

/* loop delay time in microseconds */
#define DELAY_TIME 200

int
main(int argc, char* argv[])
{
    int fd, err, rc, bitrate;
    struct termios t;
    struct timeval tv;
    unsigned char w[1], r[1];
    fd_set fds;

    if((argc != 3) || (argv[1] == "-h") || (argv[1] == "--help")) {
        printf("Usage: %s <serial device> <baudrate>\n", argv[0]);
    } else {
        printf("Opening serial device %s.\n", argv[1]);

        /* process baud rate */
        switch (atoi(argv[2])) {
            case 4800: bitrate = B4800; break;
            case 9600: bitrate = B9600; break;
            case 19200: bitrate = B19200; break;
            case 38400: bitrate = B38400; break;
            case 57600: bitrate = B57600; break;
            case 115200: bitrate = B115200; break;
            default:
                errx(1, "Unknown bitrate \"%s\"", argv[2]);
                break;
        }

        /* open the serial device in nonblocking mode */
        fd = open(argv[1], O_RDWR|O_NONBLOCK, 0);
        if(fd < 0) {
            if(errno == ENOENT) {
                printf("Serial device does not exist, waiting for it to be plugged in.\n");
                /* if the serial device isn't there, try opening it again
                 * once every 10,000 microseconds */
                while((fd = open(argv[1], O_RDWR|O_NONBLOCK, 0)) < 0) {
                    usleep(10000);
                }
            } else
                errx(1, "%s: %s", argv[1], strerror(errno));
        }

        /* get attributes and fill termios structure */
        err = tcgetattr(fd, &t);
        if(err < 0)
            errx(2, "tcgetattr: %s", strerror(errno));

        /* set input baud rate */
        err = cfsetispeed(&t, bitrate);
        if(err < 0)
            errx(3, "cfsetispeed: %s", strerror(errno));

        /* set baud rate */
        err = cfsetspeed(&t, bitrate);
        if(err < 0)
            errx(4, "cfsetspeed: %s", strerror(errno));

        /* disable canonical mode, USB key driver turns this on
         * by default */
        t.c_lflag &= ~ICANON;
        /* ignore modem status lines */
        t.c_cflag |= CLOCAL;
        err = tcsetattr (fd, TCSAFLUSH, &t);
        if (err < 0)
            errx(5, "tcsetattr: %s", strerror(errno));

        /* flush the serial device */
        tcflush(fd, TCIOFLUSH);

        w[0] = BOOTLOADER_ENTRY_CHAR;

        /* set timeout for checking readability of serial device */
        tv.tv_sec = 0;
        tv.tv_usec = DELAY_TIME;

        printf("Sending bootloader entry command (%c).\n", BOOTLOADER_ENTRY_CHAR);
        printf("You may now plug in the target device.\n");

        for(;;) {
            /* write the character to enter bootloader */
            if(write(fd, w, 1) < 1) {

                /* if this write would block, just try again */
                if (errno == EWOULDBLOCK)
                    continue;

                /* else report error */
                else
                    errx(6, "error during write: %s", strerror(errno));
            }

            /* clear file descriptor set */
            FD_ZERO(&fds);

            /* add the serial device to file descriptor set to
             * be observed */
            FD_SET(fd, &fds);

            /* check to see if there is data to be read */
            rc = select(fd+1, &fds, NULL, NULL, &tv);
            if(rc < 0)
                errx(7, "error in select");
            else {
                /* select() completed successfully */
                if(FD_ISSET(fd, &fds)) {
                    /* select() returned our serial device, so there
                     * is data to be read */
                    if(read(fd, r, 1) < 1)
                        errx(8, "error during read: %s", strerror(errno));

                    /* check if it's the right character, if not keep
                     * looping */
                    if(r[0] == BOOTLOADER_SUCCESS_CHAR) {
                        printf("Bootloader running.\n");
                        exit(0);
                    }
                }
            }
        }
    }
}
