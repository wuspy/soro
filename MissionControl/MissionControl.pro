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
    ../shared/Soro/masterarmconfig.cpp \
    ../shared/Soro/armmessage.cpp \
    ../shared/Soro/drivemessage.cpp \
    ../shared/Soro/gimbalmessage.cpp \
    ../shared/Soro/mbedchannel.cpp \
    missioncontrolprocess.cpp \
    clickablelabel.cpp \
    videoclient.cpp \
    camerawidget.cpp \
    camerawindow.cpp \
    mediaclient.cpp \
    audioclient.cpp \
    videocontrolwidget.cpp \
    audiocontrolwidget.cpp \
    audioplayer.cpp \
    ../shared/Soro/nmeamessage.cpp \
    ../shared/Soro/socketaddress.cpp \
    missioncontrolnetwork.cpp \
    gamepadmanager.cpp \
    controlsystem.cpp \
    cameracontrolsystem.cpp \
    armcontrolsystem.cpp \
    drivecontrolsystem.cpp \
    initwindow.cpp \
    mainwindow.cpp

HEADERS  += \
    ../shared/Soro/channel.h \
    ../shared/Soro/iniparser.h \
    ../shared/Soro/nmeamessage.h \
    ../shared/Soro/logger.h \
    ../shared/Soro/socketaddress.h \
    ../shared/Soro/armmessage.h \
    googlemapview.h \
    ../shared/Soro/drivemessage.h \
    ../shared/Soro/gimbalmessage.h \
    ../shared/Soro/mbedchannel.h \
    ../shared/Soro/soro_global.h \
    ../shared/Soro/masterarmconfig.h \
    missioncontrolprocess.h \
    clickablelabel.h \
    videoclient.h \
    camerawidget.h \
    camerawindow.h \
    mediaclient.h \
    audioclient.h \
    videocontrolwidget.h \
    audiocontrolwidget.h \
    audioplayer.h \
    missioncontrolnetwork.h \
    gamepadmanager.h \
    controlsystem.h \
    cameracontrolsystem.h \
    armcontrolsystem.h \
    drivecontrolsystem.h \
    initwindow.h \
    mainwindow.h

FORMS    += \
    soromainwindow.ui \
    camerawidget.ui \
    videocontrolwidget.ui \
    audiocontrolwidget.ui \
    initwindow.ui

RESOURCES += \
    Resources/MissionControl.qrc

INCLUDEPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro
DEPENDPATH += $$PWD/Resources $$PWD/../shared $$PWD/../shared/Soro

#Ubuntu dependencies: libsdl2-dev gstreamer-1.0* libqt5gstreamer*
win32: {
    LIBS += -lkernel32 -luser32 -lwinspool -lshell32 -lglu32 -lgdi32 -lopengl32
}
LIBS += -lSDL2 -lQt5GStreamer-1.0 -lQt5GLib-2.0 -lQt5GStreamerUi-1.0 -lQt5GStreamerUtils-1.0
#LIBS += -lflycapture
