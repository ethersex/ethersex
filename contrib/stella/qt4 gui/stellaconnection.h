#ifndef STELLACONNECTION_H
#define STELLACONNECTION_H

#include <QObject>
#include <QByteArray>
#include <QChar>
#include <QTimer>
#include <QHostAddress>
#include <QUdpSocket>

class stellaConnection : public QObject
{
        Q_OBJECT
public:
    stellaConnection(const QHostAddress& dest_ip, int dest_port, int source_port);
    ~stellaConnection();
    void init();
    bool validSocket();
    bool validConnection();
    int getSourcePort();
    int getDestPort();
    QString getDestIP();
    int getConnectionAttempts();
    int getBytesIn();
    int getBytesOut();
    int getProtocolVersion();
    int getChannelCount();
    int getChannelValue(unsigned char index);

    void setChannelValue(unsigned char index, unsigned char value);
    void sendBytes(const QByteArray& data);


private:
    bool valid_socket;
    bool valid_connection;
    int counter_out;          // byte counter, for statistics only
    int counter_in;           // byte counter, for statistics only
    int counter_connection;   // how many times did we try to connect
    QTimer timer_send;        // send datagrams every 20ms (if changes only)
    QTimer timer_connect;     // try to connect to stella
    QUdpSocket socket;        // socket

    QMap<unsigned char, unsigned char> changes; // every channel may save one change for the timer
    QHostAddress stella_host; // stella ipaddress (mode: channels)
    int          stella_port; // stella udp port   (mode: channels)
    int          source_port; // local port
    char protocol_version;     // response from stella: protocol version
    char channel_count;        // response from stella: channel count
    unsigned char channels[8];

private Q_SLOTS:
    // receive and send udp datagrams
    void readPendingDatagrams();
    void timer_send_timeout();
    void timer_connect_timeout();
Q_SIGNALS:
    void timer_signal();
    void established();
    void write_failure();
    void connecting();
    void received_ack();
    void channels_update();
};

#endif // STELLACONNECTION_H
