#ifndef STELLASERVER_H
#define STELLASERVER_H

#include <QtGui/QStackedWidget>
#include <QHostAddress>
#include <QAction>
#include <QTimer>
#include "pwmchannel.h"

namespace Ui {
    class StellaServer;
}
class MainWindow;
class stellaConnection;

enum StellaServerInitEnum {
    ShowChannels,
    ShowBeatcontrol
};


class StellaServer : public QStackedWidget {
    Q_OBJECT
    Q_DISABLE_COPY(StellaServer)
public:
    explicit StellaServer(const QHostAddress& ip, int port, MainWindow* mainwindow, QWidget *parent = 0);
    virtual ~StellaServer();
    void init(StellaServerInitEnum in);
    stellaConnection* getConnection();
    QList<QAction*> getActions();
    QString getName();

protected:
    virtual void changeEvent(QEvent *e);

private:
    StellaServerInitEnum inittype;
    Ui::StellaServer *ui;
    MainWindow* mainwindow;
    stellaConnection* connection;
    void makeChannelsAndLayout(int = -1);
    QVector<pwmchannel*> channels;
    QTimer timer_broadcast_changes;
    QTimer timer_beatcontrol;
    QString settings_path;    // settings path (depending on stella_host)

public Q_SLOTS:
    void timer_beatcontrol_timeout();
    void timer_signal();
    void established();
    void write_failure();
    void connecting();
    void received_ack();
    void channels_update();

private slots:
    // testcases
    void on_btnClose_2_clicked();
    void on_btnAbout_2_clicked();
    void on_spinBeat_valueChanged(int );
    void on_actionChannelCount_triggered();
    void on_actionSet_speed_rate_triggered();
    void on_actionFade_Test_4_triggered();
    void on_actionFade_Test_3_triggered();
    void on_actionFade_Test_2_triggered();
    void on_actionFade_Test_1_triggered();
    // buttons
    void on_btnAbortConnection_clicked();
    void on_predefinedlist_editTextChanged(QString );
    void on_predefinedlist_activated(QString );
    void on_btnSavePredefined_clicked();
    void on_btnRemovePredefined_clicked();
    void on_btnClose_clicked();
    void on_btnAbout_clicked();
    void on_actionResend_triggered();
    void on_actionRefetch_triggered();
    void on_btnSaveEEPROM_clicked();
    void on_btnLoadEEPROM_clicked();
    // gui logic: channels
    void value_changed(unsigned char value, unsigned char channel);
    void timer_broadcast_changes_timeout();

};

#endif // STELLASERVER_H
