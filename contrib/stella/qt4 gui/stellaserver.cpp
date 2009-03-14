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
#include "stellaserver.h"
#include "ui_stellaserver.h"
#include "mainwindow.h"
#include "stellaconnection.h"
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include "stella.h"
#include <cstdlib> //rand

StellaServer::StellaServer(const QHostAddress& ip, int port, MainWindow* mainwindow, QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::StellaServer),
    mainwindow(mainwindow)
{
    ui->setupUi(this);
    connection = new stellaConnection(ip, port, 2341);
    connect(connection, SIGNAL(timer_signal()), SLOT(timer_signal()));
    connect(connection, SIGNAL(established()), SLOT(established()));
    connect(connection, SIGNAL(write_failure()), SLOT(write_failure()));
    connect(connection, SIGNAL(connecting()), SLOT(connecting()));
    connect(connection, SIGNAL(received_ack()), SLOT(received_ack()));
    connect(connection, SIGNAL(channels_update()), SLOT(channels_update()));

    connect(&timer_beatcontrol, SIGNAL(timeout()), SLOT(timer_beatcontrol_timeout()));
    
    settings_path = QLatin1String("hosts/") + connection->getDestIP() + QLatin1String("/");

    // init broadcast timer
    timer_broadcast_changes.setSingleShot(true);
    timer_broadcast_changes.setInterval(1000); // one second after the last change broadcast changes
    connect((&timer_broadcast_changes), SIGNAL(timeout()), SLOT(timer_broadcast_changes_timeout()));

    this->setCurrentIndex(0);
}

StellaServer::~StellaServer()
{
    delete ui;
    delete connection;
}

void StellaServer::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QList<QAction*> StellaServer::getActions()
{
    QList<QAction*> actions;
    actions.append(ui->actionBroadcastChanges);
    actions.append(ui->actionChannelCount);
    actions.append(ui->actionResend);
    actions.append(ui->actionRefetch);
    return actions;
}

QString StellaServer::getName()
{
    return connection->getDestIP();
}

stellaConnection* StellaServer::getConnection()
{
    return connection;
}

void StellaServer::init(StellaServerInitEnum in)
{
    inittype = in;
    if (connection->validSocket())
    {
        connection->init();
    } else {
        ui->lblConnect->setText(tr("Could not bind to local udp socket %1").arg(connection->getSourcePort()));
    }
}

void StellaServer::on_spinBeat_valueChanged(int v)
{
    QByteArray data;
    v = 60000/v;
    // increase the fade steep on greater timer pulses
    if (v > 2000)
    {
        data.append(STELLA_FADE_STEP);
        data.append(5);
    } else
    if (v > 1500)
    {
        data.append(STELLA_FADE_STEP);
        data.append(3);
    } else
    if (v > 900)
    {
        data.append(STELLA_FADE_STEP);
        data.append(2);
    } else
    {
        data.append(STELLA_FADE_STEP);
        data.append(1);
    }
    connection->sendBytes(data);

    timer_beatcontrol.start(v);
}

void StellaServer::timer_beatcontrol_timeout()
{
    QByteArray data;
    /*
    for (char i=STELLA_SET_COLOR_0;i<STELLA_SET_COLOR_7;++i) {
        data.append(i); // only 2 byte packets allowed
        data.append((char)0);
    }
    */
    for (char i=0;i<3;++i)
    {
        if (rand() % 100<40)
        {
            //static color with p=40%
            data.append(STELLA_SET_COLOR_0+i); // only 2 byte packets allowed
            data.append((char)(rand() % 156+100));
        } else {
            data.append(STELLA_SET_COLOR_0+i); // only 2 byte packets allowed
            data.append((char)(rand() % 156+100));
        }
    }
    connection->sendBytes(data);
}

void StellaServer::timer_signal()
{
    // update channel values (fade buttons, animations)
    foreach (pwmchannel* ch, channels) ch->update();
}

void StellaServer::established()
{
    // read predefined channel sets
    QSettings settings;
    settings.beginGroup(QLatin1String("predefined"));
    QStringList predefined = settings.childGroups();
    ui->predefinedlist->clear();
    foreach (QString aset, predefined) ui->predefinedlist->addItem(aset);
    ui->predefinedlist->setCurrentIndex(-1);

    //testcases
    ui->btnTestcase->addAction(ui->actionSet_speed_rate);
    ui->btnTestcase->addAction(ui->actionFade_Test_1);
    ui->btnTestcase->addAction(ui->actionFade_Test_2);
    ui->btnTestcase->addAction(ui->actionFade_Test_3);
    ui->btnTestcase->addAction(ui->actionFade_Test_4);

    if (inittype == ShowChannels)
    {
        // make channel layout
        makeChannelsAndLayout();
        this->setCurrentIndex(1);
    }
    else if (inittype == ShowBeatcontrol)
    {
        ui->spinBeat->setValue(130);
        timer_beatcontrol.start(ui->spinBeat->value());
        this->setCurrentIndex(2);
    }

}

void StellaServer::write_failure()
{
    // still not in a connection
    if (!connection->validConnection())
    {
        ui->lblConnect->setText(tr("Write failure..."));
    }
}

void StellaServer::connecting()
{
    ui->lblConnect->setText(tr("Connecting..."));
}

void StellaServer::received_ack()
{

}

void StellaServer::channels_update()
{
    for (unsigned char i=0; i<channels.size();++i)
        channels[i]->setValue(connection->getChannelValue(i),true);
}


void StellaServer::timer_broadcast_changes_timeout() {
    QByteArray data;
    data.append(STELLA_BROADCAST_RESPONSE); // only 2 byte packets allowed
    data.append(STELLA_BROADCAST_RESPONSE);
    connection->sendBytes(data);
}

void StellaServer::on_btnLoadEEPROM_clicked()
{
    QByteArray data;
    data.append(STELLA_LOAD_FROM_EEPROM); // only 2 byte packets allowed
    data.append(STELLA_LOAD_FROM_EEPROM);
    data.append(STELLA_UNICAST_RESPONSE); // only 2 byte packets allowed
    data.append(STELLA_UNICAST_RESPONSE);
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Command 'Load from eeprom' send"));
}

void StellaServer::on_btnSaveEEPROM_clicked()
{
    QByteArray data;
    data.append(STELLA_SAVE_TO_EEPROM); // only 2 byte packets allowed
    data.append(STELLA_SAVE_TO_EEPROM);
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Command 'Save to eeprom' send"));
}

void StellaServer::makeChannelsAndLayout(int channel_count)
{
    QSettings settings;

    if (channel_count == -1)
        channel_count = settings.value(settings_path+QLatin1String("channel_count"), connection->getChannelCount()).toInt();
    else
        settings.setValue(settings_path+QLatin1String("channel_count"),channel_count);

    // get layout for removing and inserting channel ui parts
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(ui->pwmchannels->layout());
    Q_ASSERT(layout);

    // remove old channels
    foreach (QWidget* w, channels) layout->removeWidget(w);
    qDeleteAll(channels);
    channels.clear();

    // create channels (up to 8)
    for (unsigned char i=0;i<(unsigned char)channel_count;++i) {
        pwmchannel* ch = new pwmchannel(i, connection->getDestIP(), ui->pwmchannels);
        layout->insertWidget(i+1, ch);
        ch->setVisible(true);
        channels.append(ch);
        connect(ch, SIGNAL(value_changed(unsigned char,unsigned char)),
                SLOT(value_changed(unsigned char, unsigned char)));

        ch->setValue(connection->getChannelValue(i),true);
    }
}

void StellaServer::value_changed(unsigned char value, unsigned char channel)
{
    connection->setChannelValue(channel, value);

    // if the user set the corresponding option, make stella
    // broadcast all channel values 1 second after the last change
    if (ui->actionBroadcastChanges->isChecked())
        timer_broadcast_changes.start();
}

void StellaServer::on_actionRefetch_triggered()
{
        QByteArray data;
    data.append(STELLA_UNICAST_RESPONSE);
    data.append(STELLA_UNICAST_RESPONSE);
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Fetch command send"));
}

void StellaServer::on_actionResend_triggered()
{
       foreach (pwmchannel* ch, channels)
        connection->setChannelValue(ch->channel_no, ch->getValue());
    mainwindow->statusBar()->showMessage(tr("Resend all values"));
}

void StellaServer::on_btnAbout_clicked()
{
    QMessageBox::information(this, tr("About stella server"),
        tr("Server IP: %1\nServer Port: %2\nProtocol version: 1.%3\nChannels/Pins: %4"
           "\nBytes written: %5\nBytes read: %6\nConncection attempts: %7")
        .arg(connection->getDestIP())
        .arg(connection->getDestPort())
        .arg(connection->getProtocolVersion())
        .arg(connection->getChannelCount())
        .arg(connection->getBytesOut())
        .arg(connection->getBytesIn())
        .arg(connection->getConnectionAttempts()));
}

void StellaServer::on_btnClose_clicked()
{
    deleteLater();
}

void StellaServer::on_btnAbout_2_clicked()
{
    on_btnAbout_clicked();
}

void StellaServer::on_btnClose_2_clicked()
{
    on_btnClose_clicked();
}

void StellaServer::on_btnRemovePredefined_clicked()
{
    ui->btnSavePredefined->setEnabled(false);
    ui->btnRemovePredefined->setEnabled(false);

    QSettings settings;
    QString set_name = ui->predefinedlist->currentText().trimmed();
    QString path = QLatin1String("predefined/")+set_name;
    settings.remove(path);

    ui->predefinedlist->removeItem(ui->predefinedlist->currentIndex());
    ui->predefinedlist->setEditText(QString());
}

void StellaServer::on_btnSavePredefined_clicked()
{
    ui->btnSavePredefined->setEnabled(false);

    // get set name
    QString set_name = ui->predefinedlist->currentText().trimmed();
    if (set_name.size()==0) {
        mainwindow->statusBar()->showMessage(tr("Not a valid predefined channel set name!"));
        return;
    }

    // add set with set_name to the combo box if not already inserted
    if ( ui->predefinedlist->findText(set_name)==-1) ui->predefinedlist->addItem(set_name);

    // save channel values
    QSettings settings;
    QString path = QLatin1String("predefined/")+set_name+QLatin1String("/ch");
    foreach (pwmchannel* ch, channels)
        settings.setValue(path+QString::number(ch->channel_no),ch->getValue());

    mainwindow->statusBar()->showMessage(tr("Saved predefined channel set: %1").arg(set_name));
}

void StellaServer::on_predefinedlist_activated(QString set_name)
{
    ui->btnSavePredefined->setEnabled(false);
    ui->btnRemovePredefined->setEnabled(true);

    // get set name
    if (set_name.size()==0) {
        mainwindow->statusBar()->showMessage(tr("Not a valid predefined channel set name!"));
        return;
    }
    // read channel values
    QSettings settings;
    QString path = QLatin1String("predefined/")+set_name+QLatin1String("/ch");
    foreach (pwmchannel* ch, channels)
        ch->setValue((unsigned char) settings.value(path+QString::number(ch->channel_no),0).toInt());

    mainwindow->statusBar()->showMessage(tr("Loaded predefined channel set: %1").arg(set_name));
}

void StellaServer::on_predefinedlist_editTextChanged(QString )
{
    ui->btnSavePredefined->setEnabled(true);
    ui->btnRemovePredefined->setEnabled(false);
}

void StellaServer::on_btnAbortConnection_clicked()
{
    on_btnClose_clicked();
}

void StellaServer::on_actionChannelCount_triggered()
{
     bool ok;
     int value = QInputDialog::getInteger(this, tr("Set channel count"),
                                      tr("Value:"), 8, 1, channels.size(), 1, &ok);
     if (ok)
     {
         makeChannelsAndLayout(value);
     }
}

/////////////////////////////////////////////////////
/////////////////// TESTCASES ///////////////////////

/* fade all up with the fade timer from stella -> no gui update! */
void StellaServer::on_actionFade_Test_1_triggered()
{
    QByteArray data;
    for (char i=STELLA_FADE_COLOR_0;i<STELLA_FADE_COLOR_7;++i) {
        data.append(i); // only 2 byte packets allowed
        data.append(255);
    }
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Test 1"));
}

/* flash and fade all down with the fade timer from stella -> no gui update! */
void StellaServer::on_actionFade_Test_2_triggered()
{
    QByteArray data;
    for (char i=STELLA_FADE_COLOR_0;i<STELLA_FADE_COLOR_7;++i) {
        data.append(i); // only 2 byte packets allowed
        data.append((char)0);
    }
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Test 2"));
}

void StellaServer::on_actionFade_Test_3_triggered()
{
    QByteArray data;
    for (char i=STELLA_FLASH_COLOR_0;i<STELLA_FLASH_COLOR_7;++i) {
        data.append(i); // only 2 byte packets allowed
        data.append(255);
    }
    connection->sendBytes(data);
    mainwindow->statusBar()->showMessage(tr("Test 3"));
}

void StellaServer::on_actionSet_speed_rate_triggered()
{
     bool ok;
     int i = QInputDialog::getInteger(this, tr("Get fading speed rate"),
                                      tr("Value:"), 10, 1, 20, 1, &ok);
     if (ok)
     {
        QByteArray data;
        data.append(STELLA_FADE_STEP);
        data.append((unsigned char)i);
        connection->sendBytes(data);
     }
}

void StellaServer::on_actionFade_Test_4_triggered()
{
    this->setEnabled(false);
    for (int i=0;i<10;++i) {
        QByteArray data;
        for (char i=STELLA_SET_COLOR_0;i<STELLA_SET_COLOR_7;++i) {
            data.append(i); // only 2 byte packets allowed
            data.append((char)1);
        }
        for (char i=STELLA_FADE_COLOR_0;i<STELLA_FADE_COLOR_7;++i) {
            data.append(i); // only 2 byte packets allowed
            data.append((char)0);
        }
        connection->sendBytes(data);
        sleep(1);
    }
    this->setEnabled(true);
}
