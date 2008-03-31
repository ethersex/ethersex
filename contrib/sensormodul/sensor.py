#!/usr/bin/python
# -*- coding: UTF-8 -*-

# ethersex rfm12 testprogramm
import socket
import sys
import binascii, string, optparse, math, time

#HOST = 'sensor1.dyn.metafnord.de'    # ethersex host
PORT = 11587             # sensor port
s = None
CONDITION_RED = 1
CONDITION_YELLOW = 3
CONDITION_OFF = 0
CONDITION_GREEN = 2
CONDITION_NOTSET = 4
akt_ledcondition = CONDITION_NOTSET
LED = ( "OFF","RED","GREEN","YELLOW","NOTSET" )


parser = optparse.OptionParser()
parser.add_option("-v", "--verbose",
                  action="store_true", dest="verbose", default=False,
                  help="Debug output")
parser.add_option("-H", "--host", dest="host", default='sensor1.dyn.metafnord.de',
                  help="Sleep Time for the Log")
parser.add_option("-1", "--oneline",
                  action="store_true", dest="oneline", default=False,
                  help="Output all to one Line")
parser.add_option("-r", "--raw",
                  action="store_true", dest="raw", default=False,
                  help="Output Sensor raw direct data")
parser.add_option("-d", "--date",
                  action="store_true", dest="date", default=False,
                  help="Output current Date/Time")
parser.add_option("-l", "--log",
                  action="store_true", dest="log", default=False,
                  help="Run forever")
parser.add_option("-s", "--sleep", dest="sleep", default=10,
                  help="Sleep Time for the Log")
parser.add_option("-t", "--text", dest="text", help="Send Text to the LCD")

parser.add_option("-L", "--led", dest="led", help="Set LED on modul")
parser.add_option("-R", "--red", dest="max_red", help="Set max for red LED")
parser.add_option("-Y", "--yellow", dest="max_yellow", help="Set max for yellow LED")
parser.add_option("-O", "--off", dest="max_off", help="Set Max for LED off")
parser.add_option("-G", "--green", dest="max_green", help="Set max for green LED")
parser.add_option("-D", "--diff", dest="hysteresis", help="Set Hysteresis difference")

parser.add_option("-B", "--blink", dest="blink", help="Set LED blink mode")
parser.add_option("-C", "--countdown", dest="countdown", help="Set timeout for LCD blocking")
parser.add_option("-Q", "--question",action="store_true", dest="question", default=False, help="Wait for button pushed")
parser.add_option("-b", "--boot",action="store_true", dest="boot", default=False, help="Start Bootloader")


(options, args) = parser.parse_args()


for res in socket.getaddrinfo(options.host, PORT, socket.AF_UNSPEC, socket.SOCK_DGRAM):
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
s.settimeout(5)

getcountdown = 0
log=1
while log:

	while 1:
		if (not options.log):
			if (options.boot):
				print "Sende Bootstart"
				s.send("Boot")
				sys.exit(0)
			elif (options.text and str(options.text) != ""):
				print "Sende Text:%s" % str(options.text)
				s.send(":"+str(options.text))
			elif(options.led):
				print "Set LED to %s" % LED[int(options.led)]
				s.send("!"+str(options.led))
			elif(options.blink):
				print "Set blink option to %s" % LED[int(options.blink)]
				s.send("?"+str(options.blink))
			elif(options.countdown):
				print "Set timeout for lcd blocking to %03i" % int(options.countdown)
				getcountdown = int(options.countdown)
				s.send("c%03i" % int(options.countdown))
			elif(options.max_red):
				print "Set max for red to %03i" % int(options.max_red)
				s.send("1%03i" % int(options.max_red))
			elif(options.max_yellow):
				print "Set max for yellow to %03i" % int(options.max_yellow)
				s.send("2%03i" % int(options.max_yellow))
			elif(options.max_off):
				print "Set max for off to %03i" % int(options.max_off)
				s.send("3%03i" % int(options.max_off))
			elif(options.max_green):
				print "Set max for green to %03i" % int(options.max_green)
				s.send("4%03i" % int(options.max_green))
			elif(options.hysteresis):
				print "Set hysteresis to %03i" % int(options.hysteresis)
				s.send("5%03i" % int(options.hysteresis))
			elif ((not options.question) or getcountdown):
				getcountdown-=1
				s.send(" ")
		else:
			s.send(" ")

		try:
			data = s.recv(1024)
		except:
			data = ""
			sys.stdout.write(".")
			sys.stdout.flush()
			#print "Network read timeout"
		
		if(len(data) > 4):
			if(not options.log and (options.max_red or options.max_yellow or options.max_off or options.max_green or options.hysteresis or options.led or options.blink or options.countdown or options.text)):
				if (options.text):
					options.text = None
				elif (options.led):
					options.led = None
				elif (options.blink):
					options.blink = None
				elif (options.countdown):
					options.countdown = None
				elif (options.max_red):
					options.max_red = None
				elif (options.max_yellow):
					options.max_yellow = None
				elif (options.max_off):
					options.max_off = None
				elif (options.max_green):
					options.max_green = None
				elif (options.hysteresis):
					options.hysteresis = None
				data = ""
				continue
			else:
				break
	
	rawdata = ""
	for i in range(0, 32, 8):
		for i2 in range(2, 7):
			#print data[i+i2], i+i2
			rawdata = rawdata + data[i+i2]
	
	lt = time.localtime()
	
	if (options.verbose):
		print 'Received', len(data), binascii.hexlify(data)
		print data
		print lt
	if ((not options.oneline) and options.date):
		print time.strftime("Tag.Monat.Jahr Stunde:Minute:Sekunde: %d.%m.%Y %H:%M:%S", lt)
		#dprint time.strftime("Datum und Zeit: %c", lt)

	grenzwert = {}
	grenzwert["red"] = ord(data[32])
	grenzwert["yellow"] = ord(data[33])
	grenzwert["off"] = ord(data[34])
	grenzwert["green"] = ord(data[35])
	grenzwert["hysteresis"] = ord(data[36])
	countdown = ord(data[37])
	statusbyte = ord(data[38])
	buttoncounter = ord(data[39])
	
	if (options.verbose):
		print "red:%i yellow:%i off:%i green:%i hysteresis:%i status(hex):%02X" % (grenzwert["red"], grenzwert["yellow"], grenzwert["off"], grenzwert["green"], grenzwert["hysteresis"], statusbyte)
		print "countdown: %i, getcountdown %i, buttoncounter %i" % (countdown, getcountdown, buttoncounter)
	
	aT = -167.123
	bT = 0.275501
	cT = -0.000102316
	dT  = 1.92025e-08
	
	temperaturcalc = {}
	
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
		print ("Akt.Feuchte %2.1f %%" % (feuchte))
	
	Reltemp_raum = 6.1078 * math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)
	Reltemp_wand = 6.1078 * math.exp(((7.5*temperaturcalc[1])/(237.3+temperaturcalc[1]))/0.434294481903252)
	Maxfeuchte = Reltemp_wand / Reltemp_raum * 100.0
	
	Taupunkt = (237.3 * math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10)) / (7.5 -math.log(feuchte/100*6.1078*math.exp(((7.5*temperaturcalc[0])/(237.3+temperaturcalc[0]))/0.434294481903252)/6.1078,10))
	
	if(feuchte > (Maxfeuchte - (grenzwert["red"]/10))):
		akt_ledcondition = CONDITION_RED
	elif (feuchte > (Maxfeuchte - (grenzwert["yellow"]/10))):
		akt_ledcondition = CONDITION_YELLOW
	elif (feuchte > (Maxfeuchte - (grenzwert["off"]/10))):
		akt_ledcondition = CONDITION_OFF
	elif (feuchte < (Maxfeuchte - (grenzwert["green"]/10))):
		akt_ledcondition = CONDITION_GREEN
		
	
	if (not options.oneline):
		print ("Max.Feuchte %2.1f %% Taupunkt %2.1f °C LED: %s" % (Maxfeuchte, Taupunkt, LED[akt_ledcondition]))
		if (options.raw):
			print "rawdata: %s" % rawdata
			print "red:%i yellow:%i off:%i green:%i hysteresis:%i status(hex):%02X" % (grenzwert["red"], grenzwert["yellow"], grenzwert["off"], grenzwert["green"], grenzwert["hysteresis"], statusbyte)

	else:
		if (options.raw):
			if (options.date):
				timestr = time.strftime("%d.%m.%Y %H:%M:%S ", lt)
			else:
				timestr = ""
			print "%sTemp.Raum %2.1f °C Temp.Wand %2.1f °C Taupunkt %2.1f °C Feuchte %2.1f %% Max.Feuchte %2.1f %% LED:%s %s Gr:%i,%i,%i,%i h:%i s:%02X" % ( timestr,temperaturcalc[0],temperaturcalc[1], Taupunkt,feuchte,Maxfeuchte, LED[akt_ledcondition] ,rawdata, grenzwert["red"], grenzwert["yellow"], grenzwert["off"], grenzwert["green"], grenzwert["hysteresis"], statusbyte)
		else:
			if (options.date):
				timestr = time.strftime("%H:%M:%S ", lt)
			else:
				timestr = ""
			print "%sTemp.Raum %2.1f °C Temp.Wand %2.1f °C Taupunkt %2.1f °C Feuchte %2.1f %% Max.Feuchte %2.1f %% LED:%s" % ( timestr, temperaturcalc[0],temperaturcalc[1], Taupunkt,feuchte,Maxfeuchte,LED[akt_ledcondition])

	if (options.log):
		time.sleep(float(options.sleep))
	elif (options.question):
		if (buttoncounter != 0):
			print "Button gedrueckt"
			log=0
		elif ((getcountdown > 0 and countdown == 0) or getcountdown == 1):
			print "Timeout"
			sys.exit(1)
		else:
			time.sleep(5);
	else:
		log = 0

s.close()


