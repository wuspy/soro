QT += core network serialport
QT -= gui

CONFIG += c++11

TARGET = ArmNetworkInterface
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    armnetworkinterface.cpp

HEADERS += \
    armnetworkinterface.h

#libsoro

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-soro-Release/release/ -lsoro
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/debug/ -lsoro
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/ -lsoro
else:unix: LIBS += -L$$PWD/../build-soro-Release/ -lsoro

INCLUDEPATH += $$PWD/../soro
DEPENDPATH += $$PWD/../soro
