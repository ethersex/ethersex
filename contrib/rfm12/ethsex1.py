#!/usr/bin/python

# ethersex rfm12 testprogramm
import socket
import sys
import binascii, string

HOST = 'ethsex1.cb-funkshop.de'    # ethersex host
PORT = 32512              # rfm12 port
s = None
for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
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
s.settimeout(180)

if (sys.argv.__len__() < 2):
	s.send("\x05\x00\x02\xf1\x14")
else:
	text = string.ljust(sys.argv[1][0:8], 8)
	s.send("\x05\x00\x0A\xf1\x14"+text)
while 1:
	data = s.recv(1024)
	if(len(data) > 4):
		break
s.close()
print 'Received', len(data), binascii.hexlify(data)


aT = -167.123
bT = 0.275501
cT = -0.000102316
dT  = 1.92025e-08

for i in range(3, 9, 2):
	wert = ord(data[i+1]) * 256 + ord(data[i])
	if (wert < 1023):
		volt = (2.5/1023.0)* wert
		R = 2200.0/(5.0 - volt)*volt
		temperaturcalc = aT + R*( bT + R*( cT + R*dT))
		print "Temp %2.1f Wert %i" % ( temperaturcalc,wert )
i=9
wert = ord(data[i+1]) * 256 + ord(data[i])
feuchte = 100.0 / 1024.0 * wert
print ("Feuchte %i" % feuchte)


	