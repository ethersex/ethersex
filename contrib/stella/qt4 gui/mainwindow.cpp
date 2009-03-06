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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStatusBar>
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QStatusBar>
#include <cstdlib>
#include <ctime>
#include "stella.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    // init random seed
    srand ( time(NULL) );

    // init application info
    QCoreApplication::setOrganizationName(QLatin1String("Ethersex"));
    QCoreApplication::setOrganizationDomain(QLatin1String("www.ethersex.de"));
    QCoreApplication::setApplicationName(QLatin1String("Stella Control"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.0"));

    // init timers
    timer_resp_timeout.setInterval(1000); // one second timeout for fetching data
    timer_resp_timeout.setSingleShot(true);
    timer_broadcast_changes.setSingleShot(true);
    timer_broadcast_changes.setInterval(1000); // one second after the last change broadcast changes
    timer_send.setInterval(20);
    connect((&timer_send), SIGNAL(timeout()), SLOT(timer_send_data()));
    connect((&timer_resp_timeout), SIGNAL(timeout()), SLOT(timer_receive_timeout()));
    connect((&timer_broadcast_changes), SIGNAL(timeout()), SLOT(timer_broadcast_changes_timeout()));


    // init ui
    ui->setupUi(this);

    // statusbar
    permMesg = new QLabel(this);
    statInMesg = new QLabel(this);
    statOutMesg = new QLabel(this);
    ui->statusBar->addPermanentWidget(permMesg);
    ui->statusBar->addPermanentWidget(statInMesg);
    ui->statusBar->addPermanentWidget(statOutMesg);

    // connect button
    QMenu *menu = new QMenu(this);
    menu->addAction(ui->actionConnect_without_fetching_data);
    ui->btnConnect->setMenu(menu);
    ui->btnConnect->setDefaultAction(ui->actionConnect_with_fetching_data);

    // center window
    int scrn = QApplication::desktop()->screenNumber(this);
    QRect desk(QApplication::desktop()->availableGeometry(scrn));
    move((desk.width() - frameGeometry().width()) / 2,
      (desk.height() - frameGeometry().height()) / 2);

    // system tray
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionRestore);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionClose);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(this->windowIcon());
    ui->actionMinimize_to_tray->setEnabled(false);
    QSettings settings;
    ui->actionShow_tray_icon->setChecked(settings.value(QLatin1String("showTrayIcon"),false).toBool());

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    // enter mode "choose a server"
    modeChoose();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionShow_tray_icon_toggled(bool v)
{
    if (v) trayIcon->show();
    else trayIcon->hide();
    ui->actionMinimize_to_tray->setEnabled(v);
    QSettings().setValue(QLatin1String("showTrayIcon"), v);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason==QSystemTrayIcon::Trigger) {
        if (this->isVisible())
            on_actionMinimize_to_tray_triggered();
        else
            on_actionRestore_triggered();
    }
}

void MainWindow::on_actionRestore_triggered()
{
    showNormal();
}

void MainWindow::on_actionMinimize_to_tray_triggered()
{
    hide();
}

/// GUI Mode: Choose the stella server. Change gui mode with connect-button signal.
void MainWindow::modeChoose() {
    // stop all timers
    timer_send.stop();
    timer_resp_timeout.stop();

    // read stella hosts
    counter_in = 0;
    QSettings settings;
    settings.beginGroup(QLatin1String("hosts"));
    QStringList hosts = settings.childGroups();
    ui->lineIP->clear();
    foreach (QString host, hosts) ui->lineIP->addItem(host);
    ui->lineIP->setCurrentIndex(0);

    // default value
    // host part is enough here, port will be set on host part change
    if (ui->lineIP->count()==0)
        ui->lineIP->setEditText(QLatin1String("192.168.1.10"));

    // ui update
    setWindowTitle(QCoreApplication::applicationName());
    ui->menuStella_host->setEnabled(false);
    ui->btnConnect->setEnabled(true);
    ui->lineIP->setEnabled(true);
    ui->spinPort->setEnabled(true);
    ui->btnClearHistory->setEnabled(true);
    permMesg->setText(tr("Please choose a stella server!"));
    ui->label_connect_status->setText(QString());
    ui->stackedWidget->setCurrentIndex(0);
}

// if the host part of the stella server host+port is changed, update the port, too.
void MainWindow::on_lineIP_editTextChanged(QString )
{
    ui->spinPort->setValue(QSettings().value(settings_path+QLatin1String("port"), 2342).toInt());
}

/// GUI Mode: Fetch channel values from stella
bool MainWindow::modeInitialFetch() {
    // ui update
    setWindowTitle(QCoreApplication::applicationName() + tr(" - Connecting"));
    ui->label_connect_status->setText(tr(""));
    ui->menuStella_host->setEnabled(false);
    ui->btnConnect->setEnabled(false);
    ui->lineIP->setEnabled(false);
    ui->spinPort->setEnabled(false);
    ui->btnClearHistory->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(0);
    makeChannelsAndLayout();

    // init receiver socket (we want to receive udp packets from stella)
    socket.close();
    if (socket.bind(2341)) {
        ui->label_connect_status->setText(tr("Fetching values..."));
        connect(&socket, SIGNAL(readyRead()), SLOT(readPendingDatagrams()));
        return true;
    } else {
        this->statusBar()->showMessage(tr("Could not connect to udp socket 2341!"));
        return false;
    }

}

/// GUI Mode: Change pwm values. Change gui mode with choose-button signal.
void MainWindow::modeChannels() {
    // reset byte counter
    counter_out = 0;
    counter_in = 0;

    // read predefined channel sets
    QSettings settings;
    settings.beginGroup(QLatin1String("predefined"));
    QStringList predefined = settings.childGroups();
    ui->predefinedlist->clear();
    foreach (QString aset, predefined) ui->predefinedlist->addItem(aset);
    ui->predefinedlist->setCurrentIndex(0);

    // init timer
    timer_send.start();

    // ui update
    setWindowTitle(QCoreApplication::applicationName() + tr(" - %1").arg(stella_host.toString()));
    ui->menuStella_host->setEnabled(true);
    permMesg->setText(tr("Use stella server %1:%2").arg(stella_host.toString()).arg(QString::number(stella_port)));
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::readPendingDatagrams() {
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort;
    while (socket.hasPendingDatagrams()) {
        datagram.resize(socket.pendingDatagramSize());
        socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        counter_in += datagram.size();
        statInMesg->setText(tr("Received: %1 Bytes").arg(QString::number(counter_in)));

        // valid stella response?
        if (datagram.size()==9 && datagram[0] == 'S') {
            timer_resp_timeout.stop(); // stop timeout timer
            int ccount = qMin(channels.size(), datagram.size()-1);
            for (int i=0;i<ccount;++i)
                channels[i]->setValue(datagram[i+1], true);

            modeChannels();

        } else
        // ack packet
        if (datagram.size()==2 && datagram[0] == 'S') {
            qDebug() << "Ack packet received";
        } else {
            qDebug() << "Random data receiving. Size:" << datagram.size() << "Data:" << datagram.toHex();
        }
    }
}

void MainWindow::timer_receive_timeout() {
    qDebug() << "No stella response";
    modeChannels();
    // update all channels, no matter if they claim to already have the target value.
    foreach (pwmchannel* ch, channels)
        changes[ch->channel_no] = ch->getValue();
}

void MainWindow::timer_send_data() {
    // update channel values (fade buttons, animations)
    foreach (pwmchannel* ch, channels) ch->update();

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

    sendData(data);

    // we do not want the same change set again: clear
    changes.clear();
}

void MainWindow::timer_broadcast_changes_timeout() {
    sendData(STELLA_BROADCAST_RESPONSE);
}

void MainWindow::sendData(char value) {
    QByteArray data;
    data.append(value);
    sendData(data);
}

void MainWindow::sendData(const QByteArray& data) {
    // send bytearray as udp datagramm to stella
    qint64 written = socket.writeDatagram(data, stella_host, stella_port);
    if (written != data.size()) {
        this->statusBar()->showMessage(tr("Failed writting data to socket: ")+socket.errorString());
    } else {
        counter_out += written;
        statOutMesg->setText(tr("Send: %1 Bytes").arg(QString::number(counter_out)));
    }
}

void MainWindow::makeChannelsAndLayout(int channel_count)
{
    QSettings settings;

    if (channel_count == -1)
        channel_count = settings.value(settings_path+QLatin1String("channel_count"), 8).toInt();
    else
        settings.setValue(settings_path+QLatin1String("channel_count"),channel_count);

    // update spinBox but prevent it from recall this function
    channelcount_update = true;
    ui->spinChannelCount->setValue(channel_count);
    channelcount_update = false;

    // get layout for removing and inserting channel ui parts
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(ui->pwmchannels->layout());
    Q_ASSERT(layout);

    // remove old channels
    foreach (QWidget* w, channels) layout->removeWidget(w);
    qDeleteAll(channels);
    channels.clear();

    // create channels (up to 8)
    for (unsigned char i=0;i<(unsigned char)channel_count;++i) {
        pwmchannel* ch = new pwmchannel(i, stella_host.toString(), ui->pwmchannels);
        layout->insertWidget(i, ch);
        ch->setVisible(true);
        channels.append(ch);
        connect(ch, SIGNAL(value_changed(unsigned char,unsigned char)),
                SLOT(value_changed(unsigned char, unsigned char)));
    }
}

void MainWindow::value_changed(unsigned char value, unsigned char channel)
{
    changes[channel] = value;

    // if the user set the corresponding option, make stella
    // broadcast all channel values 1 second after the last change
    if (ui->actionMake_stella_broadcast_changes->isChecked())
        timer_broadcast_changes.start();
}

void MainWindow::on_actionClose_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About %1").arg(QCoreApplication::applicationName()),
        tr("Controls software called stella_lights on the ethersex firmware "
           "for embedded devices with atmel avr processors. Stella is able to "
           "generate up to 8 pwm signals.\n\nVersion: ")+QCoreApplication::applicationVersion());
}

void MainWindow::on_actionFetch_values_from_server_triggered()
{
    sendData(STELLA_UNICAST_RESPONSE);
    this->statusBar()->showMessage(tr("Fetch command send"));
}

void MainWindow::on_actionResend_pwm_values_to_stella_triggered()
{
    foreach (pwmchannel* ch, channels)
        changes[ch->channel_no] = ch->getValue();
    this->statusBar()->showMessage(tr("Resend all values"));
}

void MainWindow::on_actionChoose_triggered()
{
    modeChoose();
}

void MainWindow::on_actionConnect_with_fetching_data_triggered()
{
    // save host+port to state variables
    stella_port = ui->spinPort->value();
    bool success = stella_host.setAddress(ui->lineIP->currentText());
    settings_path = QLatin1String("hosts/") + stella_host.toString() + QLatin1String("/");

    // if port and host is valid continue
    if (success) {
        // add this host+port to the server history
        QSettings().setValue(settings_path+QLatin1String("port"), stella_port);

        // change mode
        if (modeInitialFetch()) {
            // try to fetch values or timeout and take defaults
            timer_resp_timeout.start();
            sendData(STELLA_UNICAST_RESPONSE);
        } else { // udp socket bind failed
            modeChannels();
        }
    }
}

void MainWindow::on_actionConnect_without_fetching_data_triggered()
{
    // save host+port to state variables
    stella_port = ui->spinPort->value();
    bool success = stella_host.setAddress(ui->lineIP->currentText());
    settings_path = QLatin1String("hosts/") + stella_host.toString() + QLatin1String("/");

    // if port and host is valid continue
    if (success) {
        // add this host+port to the server history
        QSettings().setValue(settings_path+QLatin1String("port"), stella_port);

        // change mode
        modeInitialFetch();
        modeChannels();
        // update all channels, no matter if they claim to already have the target value.
        foreach (pwmchannel* ch, channels) {
            ch->setValue(0);
        }
    }
}

void MainWindow::on_btnLoadPredefined_clicked()
{
    // get set name
    QString set_name = ui->predefinedlist->currentText();
    if (set_name.size()==0) {
        this->statusBar()->showMessage(tr("Not a valid predefined channel set name!"));
        return;
    }
    // read channel values
    QSettings settings;
    QString path = QLatin1String("predefined/")+set_name+QLatin1String("/ch");
    foreach (pwmchannel* ch, channels)
        ch->setValue((unsigned char) settings.value(path+QString::number(ch->channel_no),0).toInt());

    this->statusBar()->showMessage(tr("Loaded predefined channel set: %1").arg(set_name));
}

void MainWindow::on_btnSavePredefined_clicked()
{
    // get set name
    QString set_name = ui->predefinedlist->currentText();
    if (set_name.size()==0) {
        this->statusBar()->showMessage(tr("Not a valid predefined channel set name!"));
        return;
    }

    // add set with set_name to the combo box if not already inserted
    if ( ui->predefinedlist->findText(set_name)==-1) ui->predefinedlist->addItem(set_name);

    // save channel values
    QSettings settings;
    QString path = QLatin1String("predefined/")+set_name+QLatin1String("/ch");
    foreach (pwmchannel* ch, channels)
        settings.setValue(path+QString::number(ch->channel_no),ch->getValue());

    this->statusBar()->showMessage(tr("Saved predefined channel set: %1").arg(set_name));
}

void MainWindow::on_spinChannelCount_valueChanged(int value)
{
    if (!channelcount_update) makeChannelsAndLayout(value);
}
