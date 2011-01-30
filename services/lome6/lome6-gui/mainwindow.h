#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtNetwork/QTcpSocket>
#include <QQueue>
#include <QTimer>
#include <QSettings>

#define REQ_STATE 1
#define REQ_POWER 2
#define REQ_POWERL 3
#define REQ_RESET 4
#define REQ_UPTIME 5
#define REQ_LOME6UPTIME 6
#define REQ_T_CPU 7
#define REQ_T_RAM 8
#define REQ_T_PSU 9
#define REQ_T_SB 10
#define REQ_T_AIR 11
#define REQ_MAC 12
#define REQ_IP 13
#define REQ_GW 14
#define REQ_NM 15
#define REQ_NTP 16
#define REQ_VERSION 17

#define STATE_DISCONNECTED 0
#define STATE_CONNECTED 1
#define STATE_AUTHENTIFICATING 2
#define STATE_CONNECTING 3



namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    struct strQueue {
	QString sRequest;
	int iRequestType;
    };
    QQueue<strQueue> queue;
    QTimer *timer;
    QTimer *timerRefresh;
    bool bIsProcessing;
    int iCurrentRequestType;
    int iCurrentState;
    QSettings settings;

    void sendLome6Data(QString *sData);
    void sendLome6Data(QString sData);
    void successfullyConnected();
    void requestData(QString sRequest, int lastRequest);
    void updateData();
    void checkIP();

private slots:
    void on_linePort_textChanged(QString );
    void on_lineHost_textChanged(QString );
    void on_buttonRestart_clicked();
    void on_buttonReset_clicked();
    void on_buttonPowerLong_clicked();
    void on_buttonPower_clicked();
    void on_tabWidget_currentChanged();
    void on_buttonDisconnect_clicked();
    void on_buttonConnect_clicked();
    void on_checkboxAuth_stateChanged(int iChecked);
    void readReplyLome6();
    void processQueue();
    void updateDataSlot();
    void slotDisconnected();
    void slotError(QAbstractSocket::SocketError );
    void slotConnected();

};

#endif // MAINWINDOW_H
