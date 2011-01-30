#ifndef CUPTIME_H
#define CUPTIME_H

#include <QObject>
#include <QFile>
#include <QTextStream>

class cUptime : public QObject {
    Q_OBJECT

public:
    explicit cUptime(QObject *parent = 0);
    int getUptime();
    void setUptimeFile(QString sUptimeFile);
    int getUptime(QString sUptimeFile);

private:
    QString sUptimeFile;

signals:

public slots:

};

#endif // CUPTIME_H
