#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T16:01:42
#
#-------------------------------------------------

QT       += core gui network widgets serialport webenginewidgets
CONFIG   += c++11

TARGET = MissionControl
TEMPLATE = app

SOURCES += \
    ../shared/Soro/channel.cpp \
    ../shared/Soro/iniparser.cpp \
    ../shared/Soro/logger.cpp \
    ../shared/Soro/armglfwmap.cpp \
    ../shared/Soro/glfwmap.cpp \
    main.cpp \
    glfwmapdialog.cpp \
    googlemapview.cpp \
    videowindow.cpp \
    videopane.cpp \
    ../shared/Soro/driveglfwmap.cpp \
    ../shared/Soro/gimbalglfwmap.cpp \
    soromainwindow.cpp \
    sorowindowcontroller.cpp

HEADERS  += \
    ../shared/Soro/channel.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/latlng.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/armglfwmap.h \
    ../shared/Soro/glfwmap.h \
    ../shared/Soro/armmessage.h \
    ../shared/Soro/commonini.h \
    glfwmapdialog.h \
    googlemapview.h \
    videowindow.h \
    videopane.h \
    ../shared/Soro/driveglfwmap.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalglfwmap.h \
    ../shared/Soro/gimbalmessage.h \
    ../shared/Soro/serialchannel3.h \
    soromainwindow.h \
    sorowindowcontroller.h \
    ../shared/Soro/soro_global.h

FORMS    += \
    glfwmapdialog.ui \
    videopane.ui \
    videowindow.ui \
    soromainwindow.ui

RESOURCES += \
    Resources/MissionControl.qrc


INCLUDEPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro

#glfw (libglfw3 libglfw3-dev on ubuntu)

win32: {
    LIBS += -L$$PWD/../build-glfw-3.1.2.bin.WIN32/lib-vc2015/ -lglfw3
    INCLUDEPATH += $$PWD/../glfw-3.1.2/include
    DEPENDPATH += $$PWD/../glfw-3.1.2/include
    #necessary libs
    LIBS += -lkernel32 -luser32 -lwinspool -lshell32 -lglu32 -lgdi32 -lopengl32
}
macx: {
    LIBS += -L$$PWD/../build-glfw-3.1.2.bin.MACX/ -lglfw
    INCLUDEPATH += $$PWD/../glfw-3.1.2/include
    DEPENDPATH += $$PWD/../glfw-3.1.2/include
}
else:unix: LIBS += -lglfw

#vlc-qt (ppa:ntadej/tano libvlc-qt-core2 libvlc-qt-widgets2 libvlc-qt-dbg libvlc-qt-dev on ubuntu)
unix: LIBS += -lvlc -lVLCQtCore -lVLCQtWidgets
