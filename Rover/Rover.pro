QT += core network serialport
QT -= gui

CONFIG += c++11

TARGET = Rover
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../shared/Soro/channel.cpp \
    ../shared/Soro/serialinterop.cpp \
    ../shared/Soro/logger.cpp \
    ../shared/Soro/iniparser.cpp \
    roverworker.cpp \
    ../shared/Soro/serialchannel2.cpp

HEADERS += \
    ../shared/Soro/armmessage.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalmessage.h \
    ../shared/Soro/soroutil.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/channel.h \
    ../shared/Soro/serialinterop.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/commonini.h \
    roverworker.h \
    ../shared/Soro/serialchannel2.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro

