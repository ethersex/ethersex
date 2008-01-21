#!/usr/bin/python
# -*- coding: latin-1 -*-

# ethersex rfm12 testprogramm
import socket
import sys
import binascii, string, optparse, math

HOST = 'sensor1.dyn.metafnord.de'    # ethersex host
PORT = 11587             # sensor port
s = None

parser = optparse.OptionParser()
parser.add_option("-v", "--verbose",
                  action="store_true", dest="verbose", default=False,
                  help="Debug output")
(options, args) = parser.parse_args()
#if (options.verbose):

for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_DGRAM):
	af, socktype, proto, canonname, sa = res
	try:
		s = socket.socket(af, socktype, proto)
	except socket.error, msg:
		s = None
		continue
	try:
		s.connect(sa)
	except socket.error, msg:
		s.close()
		s = None
		continue
	break
if s is None:
	print 'could not open socket'
	sys.exit(1)
#052302f114
s.settimeout(10)

if (sys.argv.__len__() < 2):
	s.send(" ")
else:
	text = string.ljust(sys.argv[1][0:8], 8)
	s.send(" "+text)
while 1:
	data = s.recv(1024)
	if(len(data) > 4):
		break
s.close()

if (options.verbose):
	print 'Received', len(data), binascii.hexlify(data)
	print data


aT = -167.123
bT = 0.275501
cT = -0.000102316
dT  = 1.92025e-08

temperaturcalc = {};

for i in range(0, 2):
	irel=i*8
	wert = ord(data[irel+1]) * 256 + ord(data[irel])
	if (options.verbose):
		print wert
	if (wert < 1023):
		volt = (2.5/1023.0)* wert
		R = 2200.0/(5.0 - volt)*volt
		temperaturcalc[i] = aT + R*( bT + R*( cT + R*dT))
		print "Temp %2.1f °C Wert %i" % ( temperaturcalc[i],wert )
		

i=24
wert = ord(data[i+1]) * 256 + ord(data[i])
feuchte = 100.0 / 1024.0 * wert

print ("Feuchte %2.1f %%" % (feuchte))

Reltemp_raum = 6.1078 * math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)
Reltemp_wand = 6.1078 * math.exp(((7.5*temperaturcalc[1])/(237.3+temperaturcalc[1]))/0.434294481903252)
Maxfeuchte = Reltemp_wand / Reltemp_raum * 100.0

Taupunkt = (237.3 * math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10)) / (7.5 -math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10))

print ("Max.Feuchte %2.1f %% Taupunkt %2.1f °C" % (Maxfeuchte, Taupunkt))



