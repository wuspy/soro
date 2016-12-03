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

QT += core network
QT -= gui

CONFIG += c++11

TARGET = audio_streamer
CONFIG += console
CONFIG -= app_bundle

BUILD_DIR = ../build/audio_streamer
DESTDIR = ../bin
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
PRECOMPILED_DIR = $$BUILD_DIR

TEMPLATE = app

SOURCES += \
    audiostreamer.cpp \
    main.cpp

HEADERS += \
    audiostreamer.h

INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/../..

LIBS += -lQt5GStreamer-1.0 -lQt5GLib-2.0
LIBS += -L../lib -lsoro
LIBS += -L../lib -lsorogst
