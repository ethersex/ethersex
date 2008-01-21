#!/usr/bin/python
# -*- coding: latin-1 -*-

# ethersex rfm12 testprogramm
import socket
import sys
import binascii, string, optparse, math, time

HOST = 'sensor1.dyn.metafnord.de'    # ethersex host
PORT = 11587             # sensor port
s = None

parser = optparse.OptionParser()
parser.add_option("-v", "--verbose",
                  action="store_true", dest="verbose", default=False,
                  help="Debug output")
parser.add_option("-1", "--oneline",
                  action="store_true", dest="oneline", default=False,
                  help="Output all to one Line")
parser.add_option("-r", "--raw",
                  action="store_true", dest="raw", default=False,
                  help="Output Sensor raw direct data")
parser.add_option("-l", "--log",
                  action="store_false", dest="log", default=True,
                  help="Output all to one Line")
parser.add_option("-s", "--sleep", dest="sleep", default=10,
                  help="Sleep Time for the Log")
(options, args) = parser.parse_args()


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

log=1
while log:

	if (sys.argv.__len__() < 2):
		s.send(" ")
	else:
		text = string.ljust(sys.argv[1][0:8], 8)
		s.send(" "+text)
	while 1:
		data = s.recv(1024)
		if(len(data) > 4):
			break
	
	rawdata = ""
	for i in range(0, 32, 8):
		for i2 in range(2, 7):
			#print data[i+i2], i+i2
			rawdata = rawdata + data[i+i2]
	
	print rawdata
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
			if (not options.oneline):
				print "Temp %2.1f °C Wert %i" % ( temperaturcalc[i],wert )
			
	
	i=24
	wert = ord(data[i+1]) * 256 + ord(data[i])
	feuchte = 100.0 / 1024.0 * wert
	
	if (not options.oneline):
		print ("Feuchte %2.1f %%" % (feuchte))
	
	Reltemp_raum = 6.1078 * math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)
	Reltemp_wand = 6.1078 * math.exp(((7.5*temperaturcalc[1])/(237.3+temperaturcalc[1]))/0.434294481903252)
	Maxfeuchte = Reltemp_wand / Reltemp_raum * 100.0
	
	Taupunkt = (237.3 * math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10)) / (7.5 -math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10))
	
	if (not options.oneline):
		print ("Max.Feuchte %2.1f %% Taupunkt %2.1f °C" % (Maxfeuchte, Taupunkt))
	else:
		if (options.raw):
			print "Temp.Raum %2.1f °C Temp.Wand %2.1f °C Taupunkt %2.1f °C Feuchte %2.1f %% Max.Feuchte %2.1f %% %s" % ( temperaturcalc[0],temperaturcalc[1], Taupunkt,feuchte,Maxfeuchte,rawdata)

		else:
			print "Temp.Raum %2.1f °C Temp.Wand %2.1f °C Taupunkt %2.1f °C Feuchte %2.1f %% Max.Feuchte %2.1f %%" % ( temperaturcalc[0],temperaturcalc[1], Taupunkt,feuchte,Maxfeuchte)

	if (options.log):
		log = 0
	else:
		time.sleep(float(options.sleep))

s.close()


