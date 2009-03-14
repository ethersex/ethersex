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
#include <QInputDialog>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QStatusBar>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "stellaserver.h"
#include "stellaconnection.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    // init random seed
    srand ( time(NULL) );

    // init application info
    QCoreApplication::setOrganizationName(QLatin1String("Ethersex"));
    QCoreApplication::setOrganizationDomain(QLatin1String("www.ethersex.de"));
    QCoreApplication::setApplicationName(QLatin1String("Stella Control"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.1"));

    // init ui
    ui->setupUi(this);

    // statusbar
    permMesg = new QLabel(this);
    statInMesg = new QLabel(this);
    statOutMesg = new QLabel(this);
    ui->statusBar->addWidget(statInMesg);
    ui->statusBar->addWidget(statOutMesg);
    ui->statusBar->addPermanentWidget(permMesg);

    // connect button
    ui->btnConnect->addAction(ui->actionShowChannels);
    ui->btnConnect->addAction(ui->actionShowBeatcontrol);
    ui->btnConnect->setDefaultAction(ui->actionShowChannels);

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

    // read stella hosts
    settings.beginGroup(QLatin1String("hosts"));
    QStringList hosts = settings.childGroups();
    settings.endGroup();
    ui->lineIP->clear();
    foreach (QString host, hosts) ui->lineIP->addItem(host);
    ui->lineIP->setCurrentIndex(0);

    // default value
    // host part is enough here, port will be set on host part change
    if (ui->lineIP->count()==0)
        ui->lineIP->setEditText(QLatin1String("192.168.1.10"));

    on_tabWidget_currentChanged(ui->tabChoose);
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

void MainWindow::on_tabWidget_currentChanged(QWidget* widget)
{
    if (widget == ui->tabChoose)
    {
        // ui update
        ui->menuBar->removeAction(ui->menuStella_host->menuAction());

    } else {
        // ui update
        StellaServer* stellaserver = qobject_cast<StellaServer*>(widget);
        Q_ASSERT(stellaserver);
        ui->menuStella_host->setTitle(tr("%1").
                      arg(stellaserver->getConnection()->getDestIP()));
        ui->menuStella_host->clear();
        ui->menuStella_host->addActions(stellaserver->getActions());
        ui->menuBar->addAction(ui->menuStella_host->menuAction());
    }
}

// if the host part of the stella server host+port is changed, update the port, too.
void MainWindow::on_lineIP_editTextChanged(QString )
{
    ui->spinPort->setValue(QSettings().value(settings_path+QLatin1String("port"), 2342).toInt());
}

// return true if port and host is valid
StellaServer* MainWindow::create_stellaserver()
{
    // save host+port to state variables
    int stella_port = ui->spinPort->value();
    QHostAddress stella_host;
    settings_path = QLatin1String("hosts/") + stella_host.toString() + QLatin1String("/");
    bool success = stella_host.setAddress(ui->lineIP->currentText());
    if (success)
    {
        // add this host+port to the server history
        QSettings().setValue(settings_path+QLatin1String("port"), stella_port);
        StellaServer* stellaserver = new StellaServer(stella_host, stella_port, this, ui->tabWidget);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(stellaserver, stellaserver->getName()));
        return stellaserver;
    } else {
        return 0;
    }
}

void MainWindow::on_actionShowBeatcontrol_triggered()
{
    StellaServer* stellaserver = create_stellaserver();
    if (stellaserver) {
        stellaserver->init(ShowBeatcontrol);
    }
}

void MainWindow::on_actionShowChannels_triggered()
{
    StellaServer* stellaserver = create_stellaserver();
    if (stellaserver) {
        stellaserver->init(ShowChannels);
    }
}
