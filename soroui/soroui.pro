#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T15:37:48
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = soroui
TEMPLATE = lib

DEFINES += SOROUI_LIBRARY

SOURCES +=

HEADERS += soroui.h\
        soroui_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-soro-Release/release/ -lsoro
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/debug/ -lsoro
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/ -lsoro
else:unix: LIBS += -L$$PWD/../build-soro-Release/ -lsoro

INCLUDEPATH += $$PWD/../soro
DEPENDPATH += $$PWD/../soro
