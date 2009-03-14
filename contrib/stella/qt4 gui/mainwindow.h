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
#include <QLabel>

namespace Ui
{
    class MainWindowClass;
}
class StellaServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindowClass *ui;
    QLabel* permMesg;         // status bar permanent messages
    QLabel* statInMesg;
    QLabel* statOutMesg;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QString settings_path;    // settings path (depending on stella_host)

private Q_SLOTS:
    // tray icon
    void on_actionShowChannels_triggered();
    void on_actionShowBeatcontrol_triggered();
    void on_tabWidget_currentChanged(QWidget* );
    void on_actionShow_tray_icon_toggled(bool );
    void on_actionMinimize_to_tray_triggered();
    void on_actionRestore_triggered();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    // "program" menu
    void on_actionAbout_triggered();
    void on_actionClose_triggered();

    // gui logic: choose
    StellaServer* create_stellaserver();
    void on_lineIP_editTextChanged(QString );

};

#endif // MAINWINDOW_H
