/***************************************************************************
 *   Copyright (C) 2009 by david   *
 *   david.graeff@web.de           *
 *                                                                         *
 *   StellaControl is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   StellaControl is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with StellaControl.  If not, see <http://www.gnu.org/licenses/> *
 ***************************************************************************/
#include "stellaconnection.h"
#include "stella.h"
#include <QDebug>

stellaConnection::stellaConnection(const QHostAddress& dest_ip, int dest_port, int source_port)
{
    // init values
    this->stella_host = dest_ip;
    this->stella_port = dest_port;
    this->source_port = source_port;
    protocol_version = 0;
    channel_count = 0;

    // init timers
    timer_connect.setInterval(300); // timeout for fetching data
    timer_send.setInterval(20);
    connect((&timer_send), SIGNAL(timeout()), SLOT(timer_send_timeout()));
    connect((&timer_send), SIGNAL(timeout()), SIGNAL(timer_signal()));
    connect((&timer_connect), SIGNAL(timeout()), SLOT(timer_connect_timeout()));

    // reset byte counter
    counter_out = 0;
    counter_in = 0;
    counter_connection = 0;

    connect(&socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
    socket.close();
    valid_socket = socket.bind(source_port);
    valid_connection = false;
}

stellaConnection::~stellaConnection()
{
    // stop all timers
    timer_send.stop();
    timer_connect.stop();
}

void stellaConnection::init()
{
    emit connecting();
    timer_connect.start();
}

bool stellaConnection::validSocket()
{
    return valid_socket;
}

bool stellaConnection::validConnection()
{
    return valid_connection;
}

int stellaConnection::getSourcePort()
{
    return source_port;
}

int stellaConnection::getDestPort()
{
    return stella_port;
}

QString stellaConnection::getDestIP()
{
    return stella_host.toString();
}

int stellaConnection::getConnectionAttempts()
{
    return counter_connection;
}

int stellaConnection::getBytesIn()
{
    return counter_in;
}

int stellaConnection::getBytesOut()
{
    return counter_out;
}

int stellaConnection::getProtocolVersion()
{
    return protocol_version;
}

int stellaConnection::getChannelCount()
{
    return channel_count;
}

int stellaConnection::getChannelValue(unsigned char index)
{
    return channels[index];
}

void stellaConnection::setChannelValue(unsigned char index, unsigned char value)
{
    channels[index] = value;
    changes[index] = value;
}


void stellaConnection::readPendingDatagrams() {
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort;
    while (socket.hasPendingDatagrams()) {
        datagram.resize(socket.pendingDatagramSize());
        socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        counter_in += datagram.size();

        // valid stella response?
        if (datagram.size()==9 && datagram[0] == 'S') {
            int ccount = qMin((int)channel_count, datagram.size()-1);
            for (int i=0;i<ccount;++i)
                channels[i] = datagram[i+1];

            emit channels_update();
        } else
        // protocol_version response == connection established
        if (datagram.size()==12 && datagram[0] == 'S' && datagram[1] == 'P') {
            timer_connect.stop(); // stop timeout timer
            protocol_version = datagram[2];
            channel_count = datagram[3];
            int ccount = qMin((int)channel_count, datagram.size()-4);
            for (int i=0;i<ccount;++i)
                channels[i] = datagram[i+4];

            valid_connection = true;
            timer_send.start();
            emit established();

        } else
        // ack packet
        if (datagram.size()==2 && datagram[0] == 'S') {
            emit received_ack();
        } else
        {
            qDebug() << "Random data receiving. Size:" << datagram.size() << "Data:" << datagram.toHex();
        }
    }
}


void stellaConnection::timer_connect_timeout() {
    QByteArray data;
    data.append(STELLA_GET_PROTOCOL_VERSION);
    data.append(STELLA_GET_PROTOCOL_VERSION);
    sendBytes(data);
    ++counter_connection;
}

void stellaConnection::timer_send_timeout() {
    // no changes -> do nothing
    if (!changes.size()) return;

    // create a bytearray from changes
    QByteArray data;
    QMapIterator<unsigned char, unsigned char> i(changes);
    while (i.hasNext()) {
     i.next();
     data.append(i.key());
     data.append(i.value());
    }

    sendBytes(data);

    // we do not want the same change set again: clear
    changes.clear();
}

void stellaConnection::sendBytes(const QByteArray& data) {
    // send bytearray as udp datagramm to stella
    qint64 written = socket.writeDatagram(data, stella_host, stella_port);
    if (written != data.size()) {
        emit write_failure();
    } else {
        counter_out += written;
    }
}
