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

TARGET = research_rover
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

BUILD_DIR = ../build/research_rover
DESTDIR = ../bin
OBJECTS_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
UI_DIR = $$BUILD_DIR
PRECOMPILED_DIR = $$BUILD_DIR

SOURCES += main.cpp \
    researchroverprocess.cpp \
    mbeddataparser.cpp

HEADERS += \
    researchroverprocess.h \
    mbeddataparser.h

INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/../..

LIBS += -L../lib -lsoro
