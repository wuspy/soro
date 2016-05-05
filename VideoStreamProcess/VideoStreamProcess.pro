QT += core network
QT -= gui

CONFIG += c++11

TARGET = VideoStreamProcess
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    streamprocess.cpp \
    flycapcamera.cpp \

HEADERS += \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/videoencoding.h \
    ../shared/Soro/socketaddress.h \
    streamprocess.h \
    flycapcamera.h

INCLUDEPATH += $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/../shared $$PWD/../shared/Soro

LIBS += -lflycapture -lQt5GStreamer-1.0 -lQt5GLib-2.0 -lQt5GStreamerUi-1.0 -lQt5GStreamerUtils-1.0
