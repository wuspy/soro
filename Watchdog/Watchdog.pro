QT += core network
QT -= gui

CONFIG += c++11

TARGET = Watchdog
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    watchdogprocess.cpp

HEADERS += \
    watchdogprocess.h
