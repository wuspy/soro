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
    ../shared/Soro/socketaddress.cpp \
    gpsserver.cpp \
    ../shared/Soro/mbedchannel.cpp \
    roverprocess.cpp \
    #../shared/Soro/Rover/flycapenumerator.cpp \
    ../shared/Soro/Rover/videoserver.cpp \
    ../shared/Soro/Rover/videoserverarray.cpp \
    ../shared/Soro/Rover/mediaserver.cpp \
    ../shared/Soro/Rover/audioserver.cpp \
    ../shared/Soro/configuration.cpp \
    ../shared/Soro/nmeamessage.cpp \
    ../shared/Soro/Rover/usbcameraenumerator.cpp

HEADERS += \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/channel.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/armmessage.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalmessage.h \
    gpsserver.h \
    ../shared/Soro/mbedchannel.h \
    roverprocess.h \
    #../shared/Soro/Rover/flycapenumerator.h \
    ../shared/Soro/Rover/videoserver.h \
    ../shared/Soro/Rover/videoserverarray.h \
    ../shared/Soro/Rover/mediaserver.h \
    ../shared/Soro/Rover/audioserver.h \
    ../shared/Soro/nmeamessage.h \
    ../shared/Soro/initcommon.h \
    ../shared/Soro/Rover/usbcameraenumerator.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover

#LIBS += -lflycapture
