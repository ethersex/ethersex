#! /usr/bin/env python
# -*- coding: iso-8859-15 -*-

import socket
import time
import struct
<<<<<<< HEAD
ANY = "192.168.0.89"
SENDERPORT=6281
MCAST_ADDR = "192.168.0.91"
MCAST_PORT = 6282
senddata=bytearray(8)
senddata[0]=0x00
senddata[1]=0x01
senddata[2]=0xe0
senddata[3]=0x00
senddata[4]=0x01
senddata[5]=0x00
senddata[6]=0x00
senddata[7]=0x00
i=0
=======
ANY = "0.0.0.0"
SENDERPORT=6281
MCAST_ADDR = "192.168.0.91"
MCAST_PORT = 6282
senddata=bytearray(7)
senddata[0]=0x01
senddata[1]=0xE0
senddata[2]=0x00
senddata[3]=0x00
senddata[4]=0xAA
senddata[5]=0x55
senddata[6]=0x0F
i=0x55
>>>>>>> 5bdba682f09993cb1948518be21c3f1b499357c2
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
#The sender is bound on (0.0.0.0:1501)
sock.bind((ANY,SENDERPORT))
#Tell the kernel that we want to multicast and that the data is sent
#to everyone (255 is the level of multicasting)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 255)
<<<<<<< HEAD
sock.connect((MCAST_ADDR,MCAST_PORT))
while 1:
  try:
    time.sleep(1)
    #senddata =  bytes.fromhex("01E00100AA550")
    #result = sock.sendto(senddata, (MCAST_ADDR,MCAST_PORT) )
    sock.sendall(senddata)
    senddata[6]=i
    i=i+1
    print "Send %x"%i;
    if i>255:
      if(senddata[5]>254):
	senddata[5]=0;
      else:
	senddata[5]=senddata[5]+1
      i=0;
  except socket.error, e:
    sock.close()
=======
while 1:
  time.sleep(2)
  #senddata =  bytes.fromhex("01E00100AA550")
  sock.sendto(senddata, (MCAST_ADDR,MCAST_PORT) );
  senddata[4]=i
  i=i+1
  if i>255:
    i=0
  print "Send",i;
>>>>>>> 5bdba682f09993cb1948518be21c3f1b499357c2
#send the data “hello, world” to the multicast addr: port
#Any subscribers to the multicast address will receive this data
