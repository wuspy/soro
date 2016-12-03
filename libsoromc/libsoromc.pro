#-------------------------------------------------
#
# Project created by QtCreator 2016-11-30T19:49:41
#
#-------------------------------------------------

QT       += core network gui widgets webenginewidgets

CONFIG += c++11
TARGET = soromc
TEMPLATE = lib

DEFINES += SOROMC_LIBRARY

BUILD_DIR = ../build/libsoromc
DESTDIR = ../lib
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
PRECOMPILED_DIR = $$BUILD_DIR

SOURCES +=\
        armcontrolsystem.cpp \
        audioclient.cpp \
        audiocontrolwidget.cpp \
        audioplayer.cpp \
        cameracontrolsystem.cpp \
        camerawidget.cpp \
        camerawindow.cpp \
        clickablelabel.cpp \
        controlsystem.cpp \
        drivecontrolsystem.cpp \
        gamepadmanager.cpp \
        googlemapview.cpp \
        mediaclient.cpp \
        missioncontrolnetwork.cpp \
        videoclient.cpp \
        videocontrolwidget.cpp

HEADERS +=\
        soro_missioncontrol_global.h \
        armcontrolsystem.h \
        audioclient.h \
        audiocontrolwidget.h \
        audioplayer.h \
        cameracontrolsystem.h \
        camerawidget.h \
        camerawindow.h \
        clickablelabel.h \
        controlsystem.h \
        drivecontrolsystem.h \
        gamepadmanager.h \
        googlemapview.h \
        mediaclient.h \
        missioncontrolnetwork.h \
        videoclient.h \
        videocontrolwidget.h \
    util.h

FORMS   +=\
        audiocontrolwidget.ui \
        camerawidget.ui \
        videocontrolwidget.ui

INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/../..

LIBS += -lSDL2 -lQt5GStreamer-1.0 -lQt5GLib-2.0 -lQt5GStreamerUi-1.0 -lQt5GStreamerUtils-1.0
#LIBS += -lflycapture
LIBS += -L../lib -lsoro
