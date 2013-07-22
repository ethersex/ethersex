#!/usr/bin/env python3
import quick2wire.i2c as i2c
from quick2wire.i2c import I2CMaster, writing, reading

from time import sleep

address = 0x04

#hexstrings = ['0x76', '0x65', '0x72', '0x73', '0x69', '0x6f', '0x6e', '0x0']
while True :
 ecmdString = []
 ecmd = input("Input ECMD:")
 for ecmdChar in ecmd:
  ecmdString.append(ord(ecmdChar))

 ecmdString.append(0)
 print (ecmdString)

 with I2CMaster() as i2c:
  #i2c.transaction(writing(address,ecmdString))
  #sleep(.5)
  #read=i2c.transaction(reading(address,50))

  read_results = i2c.transaction(writing(address, ecmdString ),
  reading(address, 50))

 print(read_results)
