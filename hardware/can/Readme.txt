/*
 *
 * Copyright (c) 2016 by Michael Wagner <mw@iot-make.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */
 
Anleitung zur Integration eines CAN-Bus Controllers MCP2515 an ein Ethersex System:

a) Hardware/Software
------------------------------------------------------------------------------------
Der CAN-Bus Controller kann über zwei Möglichkeiten an den AVR angebunden 
werden - wobei MISO, MOSI, SCK und CS immer benötigt werden:

1. Ohne INT Pin (12) des MCP2515
Diese Anbindung spart einen Portpin am AVR, ist aber nicht performant da permanent
der Status des MCP2515 über SPI abgefragt werden muss. Daher nicht empfehlenswert.

2. INT PIN (12) des MCP2515 an einen Portpin (INT0/1/2) des AVR angebunden
Bei dieser Variante sind nun zwei Softwareoptionen möglich:

  2.1 Pollen der Messagebuffer des MCP2515 
  Bei dieser Variante ist regelmäßig zu prüfen ob CAN-Messages eingegangen sind. 
  Dies wird durch einen Low-Pegel am INT-Pin (12) des MCP2515 erkannt.

  2.2 Interruptgenerierung beim Empfang einer CAN-Nachricht
  Wird diese Einstellung gewählt, werden über eine Interruptroutine die eingehenden
  CAN-Nachrichten in ein FIFO Buffer eingelesen. Aus diesem können die empfangenen 
  Nachrichten geholt werden. 
  

b) Konfiguration:
------------------------------------------------------------------------------------
1. In make menuconfig 
	- IO: "CAN MCP2515" aktivieren
	- "Extended CANId" aktivieren für Extended CAN-Messages
	- "Interrupt aktivieren" für Interruptempfang

2. CS und INT definieren
In "/pinning/hardware/" in der entsprechenden *.m4 Datei des verwendeten Boards die
Zeilen
 
pin(MCP2515_INT, PD2, INPUT)

/* port the MCP2515 CS is attached to */
pin(SPI_CS_MCP2515, PB0, OUTPUT)

anfügen und die verdrahteten Pins eintragen.

Bei Interruptbetrieb in der Datei "/hardware/can/can.c" in den Zeilen 111,114 und
125 den verwendeten INT eintragen.

c) Testen:
------------------------------------------------------------------------------------
Zum Testen des CAN-Bus können die Testroutinen in der Datei can_example.c verwendet
werden.

In der Datei "/hardware/can/ Makefile" die Zeile 
	#$(CAN_SUPPORT)_SRC += hardware/can/can_example.c
auskommentieren.

Damit werden eingehende CAN-Messages auf der seriellen Schnittstelle ausgegeben. Auch
wird jede Sekunde eine Test-Message versendet.

d) Erweiterte Konfiguration
------------------------------------------------------------------------------------
Der MCP2515 CAN-Controller verfügt über zwei konfigurierbare Portpins RX0BF und RX1BF.
Diese können aus GPIO als Ausgang konfiguriert werden. Dazu im der 
"/hardware/can/can.h" den Eintrag: MCP2515_RXnBF_OUTPUT auf "1" setzen.


Viel Erfolg

Michael
