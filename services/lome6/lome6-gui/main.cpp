#include <QtGui/QApplication>
#include <QTranslator>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString(":/translation_") + locale);
    a.installTranslator(&translator);

    MainWindow w;
    w.setWindowIcon(QIcon(":/If.ico"));
    w.show();

    return a.exec();
}
