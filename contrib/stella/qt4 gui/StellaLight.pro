# -------------------------------------------------
# Project created by QtCreator 2009-02-28T18:28:31
# -------------------------------------------------
QT += network \
    dbus
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
RESOURCES += icons.qrc
OTHER_FILES += COPYING.txt
