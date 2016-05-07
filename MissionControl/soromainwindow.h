#ifndef SOROMAINWINDOW_H
#define SOROMAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QtWebEngineWidgets>
#include <QDebug>
#include <QHostAddress>
#include <QFile>
#include <QtCore/qmath.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include "channel.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "latlng.h"
#include "missioncontrolprocess.h"
#include "camerawidget.h"
#include "camerawindow.h"

namespace Ui {
    class SoroMainWindow;
}

namespace Soro {
namespace MissionControl {

    class SoroMainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit SoroMainWindow(QString name, bool masterSubnetNode, MissionControlProcess::Role role, QWidget *parent = 0);
        ~SoroMainWindow();

    private:
        Ui::SoroMainWindow *ui;
        CameraWindow *_videoWindow;
        MissionControlProcess *_controller;
        bool _fullscreen = false;
        int _initTimerId = TIMER_INACTIVE;

        void updateGamepadLabel(SDL_GameController *controller);

    signals:
        void settingsClicked();

    private slots:
        void onFatalError(QString description);
        void onWarning(QString description);
        void onGamepadChanged(SDL_GameController *controller);
        void onConnectionStateChanged(Channel::State controlChannelState, Channel::State mccNetworkState, Channel::State sharedChannelState);
        void onRttUpdate(int rtt);
        void onDroppedPacketRateUpdate(int droppedRatePercent);
        void onRoverSystemStateUpdate(RoverSubsystemState armSystemState, RoverSubsystemState driveCameraSystemState,
                                    RoverSubsystemState secondaryComputerState);
        void onRoverCameraUpdate(RoverCameraState camera1State, RoverCameraState camera2State, RoverCameraState camera3State,
                               RoverCameraState camera4State, RoverCameraState camera5State);
        void arm_onMasterArmStateChanged(MbedChannel *channel, MbedChannel::State state);
        void onNotification(MissionControlProcess::NotificationType type, QString sender, QString message);


    protected:
        void keyPressEvent(QKeyEvent *e);
        void resizeEvent(QResizeEvent *e);
        void timerEvent(QTimerEvent *e);
    };

}
}

#endif // SOROMAINWINDOW_H
