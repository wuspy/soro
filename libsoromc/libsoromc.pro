## Copyright 2016 The University of Oklahoma.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

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
