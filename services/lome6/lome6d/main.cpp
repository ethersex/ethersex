#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>
#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QSettings>

#include "lome6.h"
#include "cuptime.h"

#define ___VER___ "0.1"


// TODO exceptions geh nicht bei const char

int parseSensors(QString *sInput, QString *sRegex);

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);

    QTextStream qout(stdout);
    qout << "lome6d v"___VER___ << endl;


    QSettings settings("lome6d.ini", QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {

	qout << "Error opening lome6d.ini!" << endl;
	return -1;

    }

    int iPort = settings.value("port").toInt();
    QString sHost = settings.value("host").value<QString>();
    if (sHost == "" || iPort == 0) {

	qout << "Error opening lome6d.ini!" << endl;
	return -1;

    }
    int iAuth = settings.value("auth").toInt();
    QString sAuthUsername;
    QString sAuthPassword;
    if (iAuth == 0) {

	sAuthUsername = "";
	sAuthPassword = "";

    } else {

	sAuthUsername = settings.value("username").value<QString>();
	sAuthPassword = settings.value("password").value<QString>();

    }

    qout << "conecting to " << sHost << ":" << iPort << endl;
    if (iAuth == 1) qout << "using authentification." << endl;

    // create scoped pointer
    QScopedPointer<lome6> pLome6(NULL);

    try {

	// assign new lome6 class instance to pLome6 and initialize class
	pLome6.reset(new lome6());
	pLome6->init(sHost, iPort, sAuthUsername, sAuthPassword);

    } catch (cLome6Exception &e) {

	qout << "Exception: " << e.getExceptionText() << endl;
	return -1;

    } catch (...) {

	qout << "Exception during initialization!" << endl;
	return -1;

    }

    qout << "connected." << endl;

    while (true) {

	try {

	    // first get the uptime
	    QString sUptime = "";
	    QTextStream streamUptime(&sUptime);

	    QScopedPointer<cUptime> uptime(new cUptime());
	    int iUptime = uptime->getUptime();

	    // report uptime to lome6
	    streamUptime << "lome6 uptime " << iUptime;
	    if (!pLome6->sendCommand(sUptime)) {

		throw "Cannot send uptime.";

	    }

	} catch (const char &e) {

	    qout << "Exception: " << e << endl;

	} catch (cLome6Exception &e) {

	    qout << "Exception: " << e.getExceptionText() << endl;

	} catch (...) {

	    qout << "Unkown exception during uptime retrieval." << endl;

	}



	try {

	    // spawn a new process for lm-sensors (run sensors command)
	    QObject *parent = NULL;
	    //QProcess *sensors = new QProcess(parent);
	    QScopedPointer<QProcess> sensors(new QProcess(parent));
	    // TODO settings file
	    sensors->start("sensors");

	    // give sensors 2seconds to run
	    if (sensors->waitForFinished(2000) == false) {

		throw "sensors took too long to finish.";

	    } else {

		// get sensors output
		QByteArray byteArray = sensors->readAllStandardOutput();
		QString sOutput(byteArray.data());

		// do some regex parsing for southbridge
		QString sRegexSB;
		QTextStream streamRegexSB(&sRegexSB);
		// TODO regex from settings file
		streamRegexSB << "temp1:       \\+(.*).0°C";

		int iSBTemp = parseSensors(&sOutput, &sRegexSB);

		QString sSendSB = "";
		QTextStream streamSendSB(&sSendSB);
		streamSendSB << "lome6 set_t sb " << (iSBTemp * 10);

		if (!pLome6->sendCommand(sSendSB)) {

		    throw "cannot send sb temperature.";

		}

		// do some regex parsing for the core
		const int iCores = 16;
		int iCoreTemp[iCores];

		int iFound = 0;
		int iCPUTemp = 0;

		for (int i = 0; i < iCores; i++) {

		    QString sRegex;
		    QTextStream streamRegex(&sRegex);
		    // TODO regex from settings file
		    streamRegex << "Core" << i << " Temp:  \\+(.*).0°C";

		    iCoreTemp[i] = parseSensors(&sOutput, &sRegex);

		    if (iCoreTemp[i] > 0) {

			iFound++;
			iCPUTemp += iCoreTemp[i];

		    }

		}

		if (iFound == 0) {

		    throw "No core temperature matches found in lm-sensors output!";

		} else {

		    iCPUTemp /= iFound;

		    QString sSend = "";
		    QTextStream streamSend(&sSend);

		    streamSend << "lome6 set_t cpu " << (iCPUTemp * 10);
		    if (!pLome6->sendCommand(sSend)) {

			throw "cannot send cpu temperature.";

		    }

		}

	    }

	} catch (const char &e) {

	    qout << "Exception: " << e << endl;

	} catch (cLome6Exception &e) {

	    qout << "Exception: " << e.getExceptionText() << endl;

	} catch (...) {

	    qout << "Exception during mainloop!" << endl;

	}

	// wait 5 minutes
	sleep(300);

    }

    return a.exec();
}



// parse the sensor output
int parseSensors(QString *sInput, QString *sRegex) {

    QRegExp regex(*sRegex);

    regex.setCaseSensitivity(Qt::CaseInsensitive);
    regex.setMinimal(true);

    if (regex.indexIn(*sInput, 0) != -1) {

	QString sResult(regex.cap(1));

	QTextStream streamText(&sResult);
	int i = 0;
	streamText >> i;
	return i;

    }

    return 0;

}

