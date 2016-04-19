#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T16:01:42
#
#-------------------------------------------------

QT       += core gui network widgets webenginewidgets
CONFIG   += c++11

TARGET = MissionControl
TEMPLATE = app

SOURCES += \
    ../shared/Soro/channel.cpp \
    ../shared/Soro/iniparser.cpp \
    ../shared/Soro/logger.cpp \
    main.cpp \
    googlemapview.cpp \
    soromainwindow.cpp \
    sorowindowcontroller.cpp \
    ../shared/Soro/soroini.cpp \
    mcini.cpp \
    ../shared/Soro/masterarmconfig.cpp \
    ../shared/Soro/armmessage.cpp \
    ../shared/Soro/drivemessage.cpp \
    ../shared/Soro/gimbalmessage.cpp \
    ../shared/Soro/mbedchannel.cpp

HEADERS  += \
    ../shared/Soro/channel.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/latlng.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/armmessage.h \
    googlemapview.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalmessage.h \
    ../shared/Soro/mbedchannel.h \
    soromainwindow.h \
    sorowindowcontroller.h \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/soroini.h \
    mcini.h \
    ../shared/Soro/masterarmconfig.h

FORMS    += \
    soromainwindow.ui

RESOURCES += \
    Resources/MissionControl.qrc


INCLUDEPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro

#glfw (libglfw3-dev)

#win32: {
#    LIBS += -L$$PWD/../build-glfw-3.1.2.bin.WIN32/lib-vc2015/ -lglfw3
#    INCLUDEPATH += $$PWD/../glfw-3.1.2/include
#    DEPENDPATH += $$PWD/../glfw-3.1.2/include
#    #necessary libs
#    LIBS += -lkernel32 -luser32 -lwinspool -lshell32 -lglu32 -lgdi32 -lopengl32
#}
#macx: {
#    LIBS += -L$$PWD/../build-glfw-3.1.2.bin.MACX/ -lglfw
#    INCLUDEPATH += $$PWD/../glfw-3.1.2/include
#    DEPENDPATH += $$PWD/../glfw-3.1.2/include
#}
#else:unix: LIBS += -lglfw

#SDL (libsdl2-dev)
win32: {
    LIBS += -lkernel32 -luser32 -lwinspool -lshell32 -lglu32 -lgdi32 -lopengl32
}
LIBS += -lSDL2

#vlc-qt (ppa:ntadej/tano libvlc-qt-core2 libvlc-qt-widgets2 libvlc-qt-dbg libvlc-qt-dev on ubuntu)
#unix: LIBS += -lvlc -lVLCQtCore -lVLCQtWidgets
