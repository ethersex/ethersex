#include "lome6.h"

lome6::lome6() {

    this->sUsername = "";
    this->sPassword = "";
    tcpSocket = NULL;
    bIsConnected = false;

}


lome6::~lome6() {

    bIsConnected = false;
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
    delete tcpSocket;
    tcpSocket = NULL;

}


void lome6::init(QString sIP, int iPort, QString sUsername, QString sPassword) {

    this->sUsername = sUsername;
    this->sPassword = sPassword;

    tcpSocket = new QTcpSocket();

    tcpSocket->connectToHost(sIP, iPort);

    if (!tcpSocket->waitForConnected(5000)) {

	throw cLome6Exception("Cannot connect!");

    }

    bIsConnected = true;

}



bool lome6::sendCommand(QString sCommand) {

    if (!bIsConnected) return false;

    QTextStream streamSocket(tcpSocket);

    if (sUsername != "")
	streamSocket << "auth " << sUsername << " " << sPassword << " ";

    streamSocket << sCommand << endl;
    streamSocket.flush();

    if (!tcpSocket->waitForBytesWritten(2000)) {

	throw cLome6Exception("Cannot write!");

    }

    if (!tcpSocket->waitForReadyRead(2000)) {

	throw cLome6Exception("Cannot read!");

    }

    if (streamSocket.readLine() != "OK") return false;

    if (sUsername != "") {

	// clear password and username to prevent reauthentificating
	sUsername = "";
	sPassword = "";

    }

    return true;

}
