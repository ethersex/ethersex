#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tcpSocket = new QTcpSocket();

    QCoreApplication::setOrganizationName("ether6");
    QCoreApplication::setOrganizationDomain("http://www.ethersex.de");
    QCoreApplication::setApplicationName("lome6-gui");

    ui->lineHost->setText(settings.value("host").toString());
    ui->linePort->setText(settings.value("port").toString());
    ui->lineUsername->setText(settings.value("username").toString());
    ui->linePassword->setText(settings.value("password").toString());
    ui->checkboxAuth->setChecked(settings.value("useauth").toBool());

    checkIP();

    bIsProcessing = false;
    iCurrentRequestType = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(processQueue()));
    timer->start(10);

    timerRefresh = new QTimer(this);
    connect(timerRefresh, SIGNAL(timeout()), this, SLOT(updateDataSlot()));
    timerRefresh->start(3000);

    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));

    iCurrentState = STATE_DISCONNECTED;

}

MainWindow::~MainWindow()
{
    timer->stop();
    delete timer;

    timerRefresh->stop();
    delete timerRefresh;

    tcpSocket->disconnectFromHost();
    tcpSocket->close();
//    delete tcpSocket;

    settings.setValue("host", ui->lineHost->text());
    settings.setValue("port", ui->linePort->text());
    settings.setValue("useauth", ui->checkboxAuth->isChecked());
    settings.setValue("username", ui->lineUsername->text());
    settings.setValue("password", ui->linePassword->text());

    delete ui;
}

void MainWindow::on_checkboxAuth_stateChanged(int iChecked)
{

    if (iChecked == Qt::Checked) {

	ui->lineUsername->setEnabled(true);
	ui->linePassword->setEnabled(true);

    } else {

	ui->linePassword->setEnabled(false);
	ui->lineUsername->setEnabled(false);
	ui->lineUsername->setText("");
	ui->linePassword->setText("");

    }


}

void MainWindow::on_buttonConnect_clicked()
{

    // cancel allready opened connection
    tcpSocket->abort();

    ui->statusBar->showMessage(tr("Try connecting to ") + ui->lineHost->text() + "...");

    ui->buttonConnect->setEnabled(false);
    ui->buttonDisconnect->setEnabled(true);
    ui->lineHost->setEnabled(false);
    ui->linePort->setEnabled(false);
    ui->checkboxAuth->setEnabled(false);
    ui->lineUsername->setEnabled(false);
    ui->linePassword->setEnabled(false);

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));

    // connect to host
    tcpSocket->connectToHost(ui->lineHost->text(), ui->linePort->text().toInt());

}


void MainWindow::slotConnected() {

    ui->statusBar->showMessage(tr("Connected to ") + ui->lineHost->text() + ":" + ui->linePort->text() + tr(". Waiting for reply..."));
    sendLome6Data("mac\n");
    iCurrentState = STATE_CONNECTING;
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readReplyLome6()));

}



void MainWindow::slotDisconnected() {

    disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readReplyLome6()));
    disconnect(tcpSocket, SIGNAL(connected()), this, SLOT(slotConnected()));

    iCurrentState = STATE_DISCONNECTED;

    queue.clear();
    bIsProcessing = false;
    iCurrentRequestType = 0;

    ui->statusBar->showMessage(tr("Disconnected."));
    ui->buttonConnect->setEnabled(true);
    ui->buttonDisconnect->setEnabled(false);
    ui->lineHost->setEnabled(true);
    ui->linePort->setEnabled(true);
    ui->checkboxAuth->setEnabled(true);
    ui->tabWidget->setEnabled(false);
    ui->lineUsername->setEnabled(ui->checkboxAuth->isChecked());
    ui->linePassword->setEnabled(ui->checkboxAuth->isChecked());

}



void MainWindow::slotError(QAbstractSocket::SocketError socketError) {

    switch (socketError) {
    case QAbstractSocket::SocketTimeoutError:
	QMessageBox::critical(this, tr("Timeout"), tr("Socket timeout!"));

    case QAbstractSocket::HostNotFoundError:
	QMessageBox::critical(this, tr("Connection error"), tr("Host not found!\nYou can specify the IP address or the hostname."));
	break;

    case QAbstractSocket::NetworkError:
	QMessageBox::critical(this, tr("Network error"), tr("A network error occured.\nMaybe unknown or wrong ip address/hostname?"));
	break;

    default:
	QString sErrorNumber;
	QTextStream stx(&sErrorNumber);
	stx << " (" << socketError << ")";

	QMessageBox::critical(this, tr("Socket Error"), tr("The following socket error occured:\n") + tcpSocket->errorString() + sErrorNumber);
	break;
    }

    if (tcpSocket->isOpen()) tcpSocket->disconnectFromHost();
    slotDisconnected();

}




void MainWindow::sendLome6Data(QString *sData) {

    if (tcpSocket->waitForConnected(1)) {

	tcpSocket->write(sData->toAscii(), sData->length());

    }

}

void MainWindow::sendLome6Data(QString sData) {

    sendLome6Data(&sData);

}




void MainWindow::readReplyLome6() {

    QString sReply;
    while (tcpSocket->canReadLine())
	sReply += tcpSocket->readLine(sReply.size());

    if (iCurrentState == STATE_CONNECTED) {

	// strip trailing \n
	if (sReply.right(1) == "\n") sReply = sReply.left(sReply.length() - 1);

	QTextStream streamReply(&sReply);
	int iSeconds = 0;
	int iMinutes = 0;
	int iHours = 0;


	switch (iCurrentRequestType) {
	case REQ_STATE:
	    // for translation
	    if (sReply == "on") sReply = tr("on"); else sReply = tr("off");
	    qDebug() << sReply;
	    ui->labelSystemPowerState->setText(sReply);
	    break;

	case REQ_LOME6UPTIME:
	    ui->labelUptimeLome6->setText(sReply + "h");
	    break;

	case REQ_UPTIME:
	    streamReply >> iSeconds;
	    if (iSeconds > 0) {

		sReply = "";
		iMinutes = (iSeconds / 60) % 60;
		iHours = iSeconds / 3600;
		iSeconds = iSeconds % 60;
		streamReply << iHours << ":" << iMinutes; // << ":" << iSeconds;

		ui->labelUptime->setText(sReply + "h");

	    } else {

		ui->labelUptime->setText("-");

	    }

	    break;

	case REQ_T_CPU:
	    if (sReply == "0.0")
		ui->labelTempCPU->setText("-");
	    else
		ui->labelTempCPU->setText(sReply + "\xB0" + "C");
	    break;

	case REQ_T_AIR:
	    ui->labelTempAIR->setText(sReply + "\xB0" + "C");
	    break;

	case REQ_T_RAM:
	    ui->labelTempRAM->setText(sReply + "\xB0" + "C");
	    break;

	case REQ_T_SB:
	    if (sReply == "0.0")
		ui->labelTempSB->setText("-");
	    else
		ui->labelTempSB->setText(sReply + "\xB0" + "C");
	    break;

	case REQ_T_PSU:
	    ui->labelTempPSU->setText(sReply + "\xB0" + "C");
	    break;

	case REQ_NM:
	    ui->lineNM->setText(sReply);
	    break;

	case REQ_GW:
	    ui->lineGW->setText(sReply);
	    break;

	case REQ_IP:
	    ui->lineIP->setText(sReply);
	    break;

	case REQ_VERSION:
	    ui->labelVersion->setText(sReply);
	    break;

	case REQ_MAC:
	    ui->lineMAC->setText(sReply);
	    break;

	case REQ_NTP:
	    ui->lineNTP->setText(sReply);
	    break;

	case REQ_POWER:
	case REQ_POWERL:
	    if (sReply != "OK") QMessageBox::critical(this, tr("Warning"), tr("Power button command not successfully!"));
	    break;

	case REQ_RESET:
	    if (sReply != "OK") QMessageBox::critical(this, tr("Warning"), tr("Reset button command not successfully!"));
	    break;

	}

	bIsProcessing = false;

    } else if (iCurrentState == STATE_CONNECTING) {

	if (sReply == "authentification required\n") {

	    if (ui->checkboxAuth->isChecked()) {

		iCurrentState = STATE_AUTHENTIFICATING;
		sendLome6Data("auth " + ui->lineUsername->text().toAscii() + " " + ui->linePassword->text().toAscii() + " mac\n");
		ui->statusBar->showMessage(tr("Waiting for authentification reply..."));

	    } else {

		QMessageBox::critical(this, tr("Authentification"), tr("Authentification required!"));
		tcpSocket->disconnectFromHost();
		return;

	    }

	} else {

	    successfullyConnected();

	}

    } else if (iCurrentState == STATE_AUTHENTIFICATING) {

	if (sReply == "authentification required\n") {

	    QMessageBox::critical(this, tr("Authentification"), tr("Authentification error!\nMaybe wrong username/password?"));
	    tcpSocket->disconnectFromHost();
	    return;

	}

	successfullyConnected();

    }

}


void MainWindow::successfullyConnected() {

    iCurrentState = STATE_CONNECTED;

    ui->statusBar->showMessage(tr("Successfully connected."));

    ui->tabWidget->setEnabled(true);

    updateData();

}



void MainWindow::requestData(QString sRequest, int lastRequest) {

    strQueue putQueue;
    putQueue.iRequestType = lastRequest;
    putQueue.sRequest = sRequest + "\n";
    queue.enqueue(putQueue);

}


void MainWindow::processQueue() {

    if (!queue.isEmpty() && !bIsProcessing) {

	bIsProcessing = true;

	strQueue getQueue = queue.dequeue();

	iCurrentRequestType = getQueue.iRequestType;
	sendLome6Data(getQueue.sRequest);

    }


}



void MainWindow::updateDataSlot() {

    if (ui->tabWidget->currentIndex() != 2) updateData();

}


void MainWindow::updateData() {

    if (ui->tabWidget->isEnabled()) {

	int iIndex = ui->tabWidget->currentIndex();
	if (iIndex == 0) {

	    requestData("lome6 state", REQ_STATE);
	    requestData("whm", REQ_LOME6UPTIME);
	    requestData("lome6 uptime", REQ_UPTIME);

	} else if (iIndex == 1) {

	    requestData("lome6 get_t cpu", REQ_T_CPU);
	    requestData("lome6 get_t sb", REQ_T_SB);
	    requestData("lome6 get_t air", REQ_T_AIR);
	    requestData("lome6 get_t ram", REQ_T_RAM);
	    requestData("lome6 get_t psu", REQ_T_PSU);

	} else if (iIndex == 2) {

	    requestData("version", REQ_VERSION);
	    requestData("mac", REQ_MAC);
	    requestData("ip", REQ_IP);
	    requestData("netmask", REQ_NM);
	    requestData("gw", REQ_GW);
	    requestData("ntp server", REQ_NTP);

	}

    }

}



void MainWindow::on_buttonDisconnect_clicked()
{

    tcpSocket->abort();
    tcpSocket->disconnectFromHost();

}


void MainWindow::on_tabWidget_currentChanged()
{
    updateData();
}

void MainWindow::on_buttonPower_clicked()
{
    QMessageBox::StandardButton buttonReturn;
    buttonReturn = QMessageBox::warning(this, tr("Warning"), tr("Do you really want to press the power button?\nData loss may occure if the power button is pressed!"), QMessageBox::Yes | QMessageBox::No);
    if (buttonReturn == QMessageBox::Yes) requestData("lome6 power", REQ_POWER);
}


void MainWindow::on_buttonPowerLong_clicked()
{
    QMessageBox::StandardButton buttonReturn;
    buttonReturn = QMessageBox::warning(this, tr("Warning"), tr("Do you really want to press the power button?\nData loss may occure if the power button is pressed!"), QMessageBox::Yes | QMessageBox::No);
    if (buttonReturn == QMessageBox::Yes) requestData("lome6 power long", REQ_POWERL);
}

void MainWindow::on_buttonReset_clicked()
{
    QMessageBox::StandardButton buttonReturn;
    buttonReturn = QMessageBox::warning(this, tr("Warning"), tr("Do you really want to press the reset button?\nData loss may occure if the reset button is pressed!"), QMessageBox::Yes | QMessageBox::No);
    if (buttonReturn == QMessageBox::Yes) requestData("lome6 reset", REQ_RESET);
}

void MainWindow::on_buttonRestart_clicked()
{
    requestData("restart", 0);
}

void MainWindow::on_lineHost_textChanged(QString )
{
    checkIP();
}

void MainWindow::on_linePort_textChanged(QString )
{
    checkIP();
}


void MainWindow::checkIP() {

    int i = 0;

//    QRegExp regex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
//    if (regex.exactMatch(ui->lineHost->text())) i++;
    if (ui->lineHost->text() != "") i++;

    QRegExp regexPort("^(6553[0-5]|655[0-2]\\d|65[0-4]\\d{2}|6[0-4]\\d{3}|[1-5]\\d{4}|[1-9]\\d{0,3})$");
    if (regexPort.exactMatch(ui->linePort->text())) i++;

    if (i == 2) ui->buttonConnect->setEnabled(true); else ui->buttonConnect->setEnabled(false);

}
