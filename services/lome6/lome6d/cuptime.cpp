#include "cuptime.h"

cUptime::cUptime(QObject *parent) : QObject(parent) {

    this->sUptimeFile = "/proc/uptime";

}



void cUptime::setUptimeFile(QString sUptimeFile) {

    this->sUptimeFile = sUptimeFile;

}



int cUptime::getUptime(QString sUptimeFile) {

    this->setUptimeFile(sUptimeFile);
    return this->getUptime();

}



int cUptime::getUptime(void) {

    QFile hUptime(this->sUptimeFile);
    if (!hUptime.exists())
	throw "Uptime file does not exist!";

    if (!hUptime.open(QFile::ReadOnly))
	throw "Can not open uptime file!";

    QTextStream streamUptime(&hUptime);
    int iUptime;
    streamUptime >> iUptime;

    hUptime.close();

    return iUptime;

}
