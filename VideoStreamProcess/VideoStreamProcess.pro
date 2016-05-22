QT += core network
QT -= gui

CONFIG += c++11

TARGET = VideoStreamProcess
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    flycapcamera.cpp \
    videostreamer.cpp \
    ../shared/Soro/Rover/mediastreamer.cpp

HEADERS += \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/socketaddress.h \
    videostreamer.h \
    flycapcamera.h \
    ../shared/Soro/Rover/mediastreamer.h \
    videostreamer.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro $$PWD/../shared/Soro/Rover

LIBS += -lflycapture -lQt5GStreamer-1.0 -lQt5GLib-2.0 -lQt5GStreamerUi-1.0 -lQt5GStreamerUtils-1.0
