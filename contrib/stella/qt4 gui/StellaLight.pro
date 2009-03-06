# -------------------------------------------------
# Project created by QtCreator 2009-02-28T18:28:31
# -------------------------------------------------
QT += network
TARGET = StellaLight
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    pwmchannel.cpp
HEADERS += mainwindow.h \
    pwmchannel.h \
    stella.h
FORMS += mainwindow.ui \
    pwmchannel.ui
RESOURCES += app.qrc
RC_FILE = app.rc
OTHER_FILES += COPYING app.rc
