#!/usr/bin/python3
# Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
# Released under the GPL v3
# usage: generate.py OPTION
# OPTION:
# s: Generates a lookup table of the Stevens' power law to be copy pasted directly in to c/cpp source code
# c: Generates a lookup table of the CIELUV (CIE 1931) color space to be copy pasted directly in to c/cpp source code
#
# For more information, see
# (1) http://en.wikipedia.org/wiki/CIELUV
# (2) http://en.wikipedia.org/wiki/Stevens_power_law
#
import math
import sys
pwm_size=4096.00
pwm_levels=255

def printUsage():
  print('''usage: generate.py OPTION
 OPTION: 
 s: Generates a lookup table of the Stevens\' power law to be copy pasted directly in to c/cpp source code
 c: Generates a lookup table of the CIELUV (CIE 1931) color space to be copy pasted directly in to c/cpp source code

 For more information, see
 (1) http://en.wikipedia.org/wiki/CIELUV
 (2) http://en.wikipedia.org/wiki/Stevens_power_law
''')

def stevensPowerLaw():
  print('datatype stevens_power[%d] = {' % (pwm_levels+1), end=" ")
  for i in range(0,pwm_levels+1):
    result = math.floor(pwm_size * (i / pwm_levels)**2.2 + 0.5)
    if i < pwm_levels:
      if ((i%10 == 0) and (i > 0)):
        print('%d,' % result)
      else:
        print('%d,' % (result), end=" ")
    else:
      print('%d };' % result)
def cieLuminance():
  print('datatype cie_luminance[%d] = {' % (pwm_levels+1), end=" ")
  for i in range(0,pwm_levels+1):
    yynratio = i/pwm_levels
    if yynratio > 0.008856:
      result = ((yynratio*100+16)/116)**3
    else:
      result = (yynratio)*100/903.3
    result = result*pwm_size
    if i < pwm_levels:
      if ((i%10 == 0) and (i > 0)):
        print('%d,' % result)
      else:
        print('%d,' % (result), end=" ")
    else:
      print('%d };' % result)

if len(sys.argv) < 2:
  printUsage()
  sys.exit(0)
if sys.argv[1] == "s":
  stevensPowerLaw()
elif sys.argv[1] == "c":
  cieLuminance()
else:
  printUsage()
sys.exit(0)
