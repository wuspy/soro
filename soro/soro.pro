#-------------------------------------------------
#
# Project created by QtCreator 2016-01-26T22:21:06
#
#-------------------------------------------------

QT       += network gui

TARGET = soro
TEMPLATE = lib

DEFINES += SORO_LIBRARY

SOURCES += \
    channel.cpp \
    logger.cpp \
    tagvalueparser.cpp

HEADERS +=\
        soro_global.h \
    channel.h \
    logger.h \
    tagvalueparser.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
