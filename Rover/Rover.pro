QT += core network
QT -= gui

CONFIG += c++11

TARGET = Rover
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../shared/Soro/channel.cpp \
    ../shared/Soro/logger.cpp \
    ../shared/Soro/iniparser.cpp \
    ../shared/Soro/soroini.cpp \
    gpsserver.cpp \
    ../shared/Soro/mbedchannel.cpp \
    roverprocess.cpp \
    flycapenumerator.cpp \
    videoserver.cpp \
    uvdcameraenumerator.cpp

HEADERS += \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/channel.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/soroini.h \
    ../shared/Soro/armmessage.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalmessage.h \
    gpsserver.h \
    ../shared/Soro/mbedchannel.h \
    roverprocess.h \
    ../shared/Soro/videoencoding.h \
    flycapenumerator.h \
    videoserver.h \
    uvdcameraenumerator.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro

LIBS += -lflycapture
