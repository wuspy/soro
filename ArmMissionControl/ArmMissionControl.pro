#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T16:01:42
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ArmMissionControl
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

#libsoro include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-soro-Release/release/ -lsoro
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/debug/ -lsoro
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soro-Debug/ -lsoro
else:unix: LIBS += -L$$PWD/../build-soro-Release/ -lsoro

INCLUDEPATH += $$PWD/../soro
DEPENDPATH += $$PWD/../soro

#libsoroui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-soroui-Release/release/ -lsoroui
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soroui-Debug/debug/ -lsoroui
else:unix:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-soroui-Debug/ -lsoroui
else:unix: LIBS += -L$$PWD/../build-soroui-Release/ -lsoroui

INCLUDEPATH += $$PWD/../soroui
DEPENDPATH += $$PWD/../soroui

#glfw

win32: LIBS += -L$$PWD/../glfw-3.1.2.bin.WIN32/lib-mingw/ -lglfw3
else:unix: LIBS += -L$$PWD/../glfw-3.1.2.bin.WIN32/lib-mingw/ -lglfw3

INCLUDEPATH += $$PWD/../glfw-3.1.2.bin.WIN32/include
DEPENDPATH += $$PWD/../glfw-3.1.2.bin.WIN32/include

#needed for glfw

LIBS += -lgdi32 -lopengl32
