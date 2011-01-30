#ifndef LOME6_H
#define LOME6_H
#include <QtNetwork/QTcpSocket>

class lome6 : public QObject {
public:
    explicit lome6();
    ~lome6();
    void init(QString sIP, int iPort, QString sUsername, QString sPassword);
    bool sendCommand(QString sCommand);

private:
    QTcpSocket *tcpSocket;
    QString sUsername, sPassword;
    bool bIsConnected;

};


class cLome6Exception {
public:
    cLome6Exception(QString sErrorString) : sExceptionText(sErrorString) { };
    ~cLome6Exception() { };
    QString getExceptionText() { return this->sExceptionText; }

private:
    QString sExceptionText;

};

#endif // LOME6_H
