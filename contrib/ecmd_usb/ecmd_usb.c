/**********************************************************
 * Copyright (C) 2006 by Jochen Roessner <jochen@lugrot.de>, Germany
 * Copyright (C) 2008 by Christian Dietrich <stettberger@dokucode.de
 * Copyright (C) 2011 by Martin Wache <M.Wache@gmx.net>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* A little Wrapper around libusb to use the usb ecmd feature of ethersex */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <usb.h>
#include <errno.h>

#define USB_REQUEST_ECMD 0

  usb_dev_handle *
usb_find(const char *device)
{
  struct usb_bus *bus;
  unsigned int idVendor;
  unsigned int idProduct;
  if (sscanf(device, "%04x%04x", &idVendor, &idProduct) == 2)
  {
    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next)
    {
      struct usb_device *dev;
      for (dev = bus->devices; dev; dev = dev->next)
      {
        usb_dev_handle *usb_bus_dev;
        usb_bus_dev = usb_open(dev);
        if (usb_bus_dev)
        {
          if (dev->descriptor.idVendor == idVendor &&
              dev->descriptor.idProduct == idProduct)
          {
            printf ("gefunden! devnr: %i %04X - %04X\n",1,
                    dev->descriptor.idVendor, dev->descriptor.idProduct);
            return usb_bus_dev;
          }
        }
        if (usb_bus_dev)
          usb_close(usb_bus_dev);
      }
    }
  }
  fprintf(stderr, "Kein passendes USB Device gefunden\n");
  return 0;
}

typedef union usbWord {
uint8_t bytes[2];
uint16_t word;
} usbWord_t;

int
usb_recv(usb_dev_handle *handle,uint8_t cmd_count, uint8_t reply_count,
         char *buf, int len)
{
    usbWord_t index;
    index.bytes[1] = cmd_count;
    index.bytes[0] = reply_count;
    int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                              USB_ENDPOINT_IN,
                              USB_REQUEST_ECMD, 0,
                              index.word,
                              (char*)buf, len, 500);
    if ( ret > 0 )
      buf[ret++] = 0;
    return ret; /* > 0 = ok */
}



int
usb_send(usb_dev_handle * handle,uint8_t cmd_count, uint8_t reply_count,
         const unsigned char *data, int len)
{
  usbWord_t index;
  index.bytes[1] = cmd_count;
  index.bytes[0] = reply_count;
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                            USB_ENDPOINT_OUT,
                            USB_REQUEST_ECMD, 1,
                            index.word,
                            (char*) data, len, 500);
  if ( ret != len )
    return 0;
  return ret; /* > 0 = ok */
}

int
main() {
  usb_dev_handle *handle = usb_find("16c005dc");
  if (handle == 0) return -1;

  /* Start the Shell */
  unsigned char line[100];
  int len=0;
  struct timeval tv;
  fd_set fds;
  int no_activity=0;
  uint8_t cmd_count=0;
  uint8_t reply_count=0;
  uint64_t usec_sleep = 100000;
  while (!feof(stdin) || len != 0) {
    FD_ZERO(&fds);
    FD_SET(fileno(stdin), &fds);
    tv.tv_sec = 0;
    tv.tv_usec = usec_sleep;

    while ( ( len ==0 || line[len-1] != '\n' ) &&
            select(1, &fds, NULL, NULL, &tv) > 0 &&
            FD_ISSET(fileno(stdin), &fds) )
    {
      /* while data available read until end of line character */
      unsigned char c;
      if ( read(fileno(stdin),&c, 1 ) <= 0 )
      {
        fprintf(stderr, "Error reading from STDIN\n");
        exit(EXIT_FAILURE);
      }
      line[len++] = (unsigned char)c;
      no_activity = 0;

      FD_ZERO(&fds);
      FD_SET(fileno(stdin), &fds);
      tv.tv_sec = 0;
      tv.tv_usec = usec_sleep;
    }
    line[len]=0;

    if ( len != 0 && line[len-1] == '\n' )
    {
      /* full line available in buffer, send to device */
      int ret = usb_send(handle, cmd_count, reply_count, line, len+1 );
      if ( ret == 0 )
      {
        /* the device didn't accept the message = busy */
        usleep(100000);
      }
      if ( ret < 0 )
        printf("usb_send ret %d\n", ret);
    }

    /* check if the device has something to read */
    char buf[100];
    int ret = usb_recv(handle, cmd_count, reply_count, buf, sizeof(buf));
    if (ret > 0) {
      if ( buf[0] == cmd_count )
      {
        /* device returned cmd token, we can send the next command */
        cmd_count++;
        len=0;
      };
      if ( buf[1] == reply_count )
        /* already received this reply, ignore it */
        continue;
      /* remember reply token, to confirm the reception of the reply */
      reply_count=buf[1];

      printf("%s\n", buf+2);
      no_activity=0;
    }
    else if (ret == -ETIME || ret == -ETIMEDOUT )
    {
      /* time out try again later */
      usleep(100000);
    }
    else if ( ret<0 )
    {
      fprintf(stderr,"usb_recv return %d\n", ret);
      exit(EXIT_FAILURE);
    }

    if ( no_activity > 50 )
    {
      /* no activity for some time, time to relax a bit */
      usec_sleep = 500000;
    }
    else
    {
      usec_sleep = 10000;
      no_activity++;
    }
  }
  usb_close(handle);
}



#if 0
/* read data from i2c device
 */
int
i2c_recv(struct i2cdev *handle, unsigned char *buf, int len)
{
    int ret = -1;
    int aktlen = len;
    int lckfd;
    int maxtry = 0;
    do
    {
        lckfd = open(LCKFILE, O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
        usleep(10000);
    }
    while( lckfd < 0 && maxtry++ < 200);
    if(handle != NULL && maxtry < 200)
    {
        ret = 0;
        do
        {
            ret += usb_control_msg(handle->usbdev, USB_TYPE_VENDOR | USB_RECIP_DEVICE |  USB_ENDPOINT_IN, USBI2C_READ, 0, handle->addr, (char*)buf+len-aktlen, aktlen, 500);
            aktlen -= 255;
        }
        while(aktlen > 0);
        close(lckfd);
        unlink(LCKFILE);
    }
    return ret; /* > 0 = ok */
}

#endif
