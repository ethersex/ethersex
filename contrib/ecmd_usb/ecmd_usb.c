/**********************************************************
 * Copyright (C) 2006 by Jochen Roessner <jochen@lugrot.de>, Germany
 * Copyright (C) 2008 by Christian Dietrich <stettberger@dokucode.de

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

int
usb_recv(usb_dev_handle *handle, char *buf, int len)
{
    int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE |  USB_ENDPOINT_IN, 
                            USB_REQUEST_ECMD, 0, 0, (char*)buf, len, 500);
    if (ret != len) 
      return 0;
    return ret; /* > 0 = ok */
}



int
usb_send(usb_dev_handle * handle, const char *data, int len)
{
  int ret = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | 
                            USB_ENDPOINT_OUT, USB_REQUEST_ECMD, 1, 0, (char*) data, len, 500);
  if ( ret != len )
    return 0;
  return ret; /* > 0 = ok */
}

int 
main() {
  usb_dev_handle *handle = usb_find("16c005dc");
  if (handle == 0) return -1;

  /* Start the Shell */
  char *line = NULL;
  size_t len;
  while (getline(&line, &len, stdin) != -1) {
    line[strlen(line) - 1] = 0;
    if (usb_send(handle, line, strlen(line) + 1) == 0) {
      fprintf(stderr, "Communication failed\n");
      usb_close(handle);
      exit(EXIT_FAILURE);
    }
    /* Now we must receive the answer */
    char buf[100];
    if (usb_recv(handle, buf, sizeof(buf)) == 0) {
      fprintf(stderr, "Communication failed\n");
      usb_close(handle);
      exit(EXIT_FAILURE);
    }
    printf("%s\n", buf);
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
