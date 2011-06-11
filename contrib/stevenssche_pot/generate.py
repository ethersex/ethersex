#!/usr/bin/python3
# Copyright (c) 2011 by Maximilian Güntner <maximilian.guentner@gmail.com>
# Released under the GPL v3
# Generate a lookup table of the Stevens' power law to be copy pasted directly in to c/cpp source code
#
import math
import sys

pwm_size=4096.00
pwm_levels=255
print('datatype stevens_power[%d] = {' % (pwm_levels+1), end=" ")
for i in range(0,pwm_levels+1):
    result = math.floor(pwm_size * math.pow((i / pwm_levels),2.2) + 0.5)
    if i < pwm_levels:
       if ((i%10 == 0) and (i > 0)):
          print('%d,' % result)
       else:
          print('%d,' % (result), end=" ")
    else:
       print('%d };' % result)
