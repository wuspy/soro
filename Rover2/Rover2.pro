QT += core network
QT -= gui

CONFIG += c++11

TARGET = Rover2
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../shared/Soro/iniparser.cpp \
    ../shared/Soro/channel.cpp \
    ../shared/Soro/logger.cpp \
    ../shared/Soro/socketaddress.cpp \
    #../shared/Soro/Rover/flycapenumerator.cpp \
    ../shared/Soro/Rover/usbcameraenumerator.cpp \
    ../shared/Soro/Rover/videoserver.cpp \
    rover2process.cpp \
    ../shared/Soro/Rover/videoserverarray.cpp \
    ../shared/Soro/Rover/mediaserver.cpp \
    ../shared/Soro/configuration.cpp

HEADERS += \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/channel.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/socketaddress.h \
    #../shared/Soro/Rover/flycapenumerator.h \
    ../shared/Soro/Rover/usbcameraenumerator.h \
    ../shared/Soro/Rover/videoserver.h \
    rover2process.h \
    ../shared/Soro/Rover/videoserverarray.h \
    ../shared/Soro/Rover/mediaserver.h \
    ../shared/Soro/configuration.h \
    ../shared/Soro/initcommon.h


INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover

#LIBS += -lflycapture
