QT += core network
QT -= gui

CONFIG += c++11

TARGET = AudioStreamProcess
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ../shared/Soro/Rover/mediastreamer.cpp \
    audiostreamer.cpp

HEADERS += \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/Rover/mediastreamer.h \
    audiostreamer.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover

LIBS += -lQt5GStreamer-1.0 -lQt5GLib-2.0 -lQt5GStreamerUi-1.0 -lQt5GStreamerUtils-1.0
