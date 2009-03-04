#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTimer>
#include <QByteArray>
#include <QMap>
#include <QUdpSocket>
#include <QHostAddress>
#include <QSystemTrayIcon>
#include <QMenu>
#include "pwmchannel.h"

namespace Ui
{
    class MainWindowClass;
}

#define CHANNELCOUNT 6

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;
    int counter;              // byte counter, for statistics only
    QTimer timer_send;        // send datagrams every 20ms (if changes only)
    QTimer timer_resp_timeout;// timeout on receiving channel infos from stella
    QUdpSocket socket;        // socket
    QMap<unsigned char, unsigned char> changes; // every channel may save one change for the timer
    QHostAddress stella_host; // stella ipaddress (mode: channels)
    int          stella_port; // stella udp port   (mode: channels)
    QString settings_path;    // settings path (depending on stella_host)
    void modeChoose();        // change to mode "choose"
    void modeInitialFetch();  // change to mode "initial channel info fetch"
    void modeChannels();      // change to mode "channels"
    void askStellaForValues();// use response function of stella
    void makeChannelsAndLayout(int = -1);
    bool channelcount_update; // don't rebuild layout on spinBox value change
    QVector<pwmchannel*> channels;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

private Q_SLOTS:
    // tray icon
    void on_actionShow_tray_icon_toggled(bool );
    void on_actionMinimize_to_tray_triggered();
    void on_actionRestore_triggered();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    // receive and send udp datagrams
    void readPendingDatagrams();
    void on_timer_send();
    void on_timer_receive_timeout();

    // "program" menu
    void on_actionAbout_triggered();
    void on_actionClose_triggered();

    // "host" menu
    void on_actionChoose_triggered();
    void on_actionFetch_values_from_server_triggered();

    // predefined list
    void on_btnSavePredefined_clicked();
    void on_btnLoadPredefined_clicked();
    void on_spinChannelCount_valueChanged(int);

    // gui logic: choose
    void on_btnConnect_clicked();
    void on_lineIP_editTextChanged(QString );

    // gui logic: channels
    void value_changed(unsigned char value, unsigned char channel);
};

#endif // MAINWINDOW_H
