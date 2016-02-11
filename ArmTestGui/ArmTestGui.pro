#-------------------------------------------------
#
# Project created by QtCreator 2016-02-09T13:54:07
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ArmTestGui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

#libsoro

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-soro-Release/release/ -lsoro
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/debug/ -lsoro
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/ -lsoro
else:unix: LIBS += -L$$PWD/../build-soro-Release/ -lsoro

INCLUDEPATH += $$PWD/../soro
DEPENDPATH += $$PWD/../soro

#glfw

win32: LIBS += -L$$PWD/../glfw-3.1.2.bin.WIN32/lib-mingw/ -lglfw3
else:unix: LIBS += -L$$PWD/../glfw-3.1.2.bin.WIN32/lib-mingw/ -lglfw3

INCLUDEPATH += $$PWD/../glfw-3.1.2.bin.WIN32/include
DEPENDPATH += $$PWD/../glfw-3.1.2.bin.WIN32/include

#needed for glfw

LIBS += -lgdi32 -lopengl32
