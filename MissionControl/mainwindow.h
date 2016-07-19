#ifndef SORO_MISSIONCONTROL_MAINWINDOW_H
#define SORO_MISSIONCONTROL_MAINWINDOW_H

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
#include <QCloseEvent>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include "armmessage.h"
#include "channel.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "nmeamessage.h"
#include "camerawidget.h"
#include "camerawindow.h"
#include "videocontrolwidget.h"
#include "controlsystem.h"
#include "gamepadmanager.h"
#include "missioncontrolnetwork.h"
#include "armcontrolsystem.h"
#include "drivecontrolsystem.h"
#include "cameracontrolsystem.h"

namespace Ui {
    class SoroMainWindow;
}

namespace Soro {
namespace MissionControl {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const Configuration *config, GamepadManager *gamepad,
                            MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QWidget *parent = 0);
    ~MainWindow();

    CameraWidget* getTopCameraWidget();
    CameraWidget* getBottomCameraWidget();
    CameraWidget* getFullscreenCameraWidget();

    bool isMuteAudioSelected();

private:
    Ui::SoroMainWindow *ui;
    CameraWindow *_videoWindow;

    const Configuration *_config;
    MissionControlNetwork *_mcNetwork;
    GamepadManager *_gamepad;
    ControlSystem *_controlSystem;
    bool _fullscreen = false;
    QMovie *_preloaderMovie;
    static const QString _logLevelFormattersHTML[4];
    Channel::State _lastControlChannelState;
    Channel::State _lastRoverChannelState;
    RoverSubsystemState _lastArmSubsystemState;
    RoverSubsystemState _lastDriveCameraSubsystemState;
    RoverSubsystemState _lastSecondaryComputerState;
    int _lastDroppedPacketPercent = 0;
    int _lastRtt = 0;

    int _clearGpsStatusTimerId = TIMER_INACTIVE;

    QMessageBox *_messageBoxHolder = NULL;

signals:
    void closed();
    void cycleVideosClockwise();
    void cycleVideosCounterclockwise();
    void cameraFormatChanged(int camera, VideoFormat format);
    void audioStreamFormatChanged(AudioFormat format);
    void audioStreamMuteChanged(bool mute);
    void cameraNameEdited(int camera, QString name);

public slots:
    void onFatalError(QString description);
    void onWarning(QString description);
    void onLocationUpdate(const NmeaMessage& location);
    void onRoverChannelStateChanged(Channel::State state);
    void onRttUpdate(int rtt);
    void onBitrateUpdate(quint64 bpsRoverDown, quint64 bpsRoverUp);
    void onDroppedPacketRateUpdate(int droppedRatePercent);
    void onArmSubsystemStateChanged(RoverSubsystemState state);
    void onDriveCameraSubsystemStateChanged(RoverSubsystemState state);
    void onSecondaryComputerStateChanged(RoverSubsystemState state);
    void onMasterArmStateChanged(bool connected);
    void onMasterArmUpdate(const char *armMessage);
    void onCameraFormatChanged(int camera, VideoFormat format);
    void onAudioFormatChanged(AudioFormat format);
    void setCameraName(int camera, QString name);

private slots:
    void onGamepadChanged(SDL_GameController *controller, QString name);
    void onControlChannelStateChanged(Channel *channel, Channel::State state);
    void updateConnectionStateInformation();
    void updateSubsystemStateInformation();
    void reloadMasterArmClicked();
    void camera1ControlOptionChanged(VideoFormat option);
    void camera2ControlOptionChanged(VideoFormat option);
    void camera3ControlOptionChanged(VideoFormat option);
    void camera4ControlOptionChanged(VideoFormat option);
    void camera5ControlOptionChanged(VideoFormat option);
    void cameraControlOptionChanged(int camera, VideoFormat option);
    void camera1NameEdited(QString newName);
    void camera2NameEdited(QString newName);
    void camera3NameEdited(QString newName);
    void camera4NameEdited(QString newName);
    void camera5NameEdited(QString newName);

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);
};

}
}

#endif // SORO_MISSIONCONTROL_MAINWINDOW_H
