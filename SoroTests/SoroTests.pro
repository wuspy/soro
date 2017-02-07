#-------------------------------------------------
#
# Project created by QtCreator 2017-02-07T15:27:08
#
#-------------------------------------------------

QT       += testlib core gui network widgets webengine

TARGET = tst_sorotests
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_sorotests.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/../..

LIBS += -L../lib -lsoro
LIBS += -L../lib -lsorogst
LIBS += -L../lib -lsoromc
