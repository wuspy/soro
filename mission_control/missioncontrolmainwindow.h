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

#include "libsoro/constants.h"
#include "libsoro/enums.h"
#include "libsoro/videoformat.h"
#include "libsoro/armmessage.h"
#include "libsoro/channel.h"
#include "libsoro/mbedchannel.h"
#include "libsoro/nmeamessage.h"
#include "libsoro/audioformat.h"

#include "libsoromc/camerawidget.h"
#include "libsoromc/camerawindow.h"
#include "libsoromc/videocontrolwidget.h"
#include "libsoromc/controlsystem.h"
#include "libsoromc/gamepadmanager.h"
#include "libsoromc/missioncontrolnetwork.h"
#include "libsoromc/armcontrolsystem.h"
#include "libsoromc/drivecontrolsystem.h"
#include "libsoromc/cameracontrolsystem.h"

namespace Ui {
    class MissionControlMainWindow;
}

namespace Soro {
namespace MissionControl {

class MissionControlMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MissionControlMainWindow(GamepadManager *gamepad, MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QWidget *parent = 0);
    ~MissionControlMainWindow();

    CameraWidget* getTopCameraWidget();
    CameraWidget* getBottomCameraWidget();
    CameraWidget* getFullscreenCameraWidget();
    void setAvailableVideoFormats(QList<VideoFormat> formats);
    bool isMuteAudioSelected() const;

private:
    Ui::MissionControlMainWindow *ui;
    CameraWindow *_videoWindow;

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

    QMessageBox *_messageBoxHolder = nullptr;

signals:
    void closed();
    void cycleVideosClockwise();
    void cycleVideosCounterclockwise();
    void cameraFormatChanged(int camera, int formatIndex);
    void playAudioSelected();
    void stopAudioSelected();
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
    void onCameraFormatChanged(int camera, int formatIndex);
    void onAudioPlaying();
    void onAudioStopped();
    void setCameraName(int camera, QString name);

private slots:
    void onGamepadChanged(bool connected, QString name);
    void onControlChannelStateChanged(Channel::State state);
    void updateConnectionStateInformation();
    void updateSubsystemStateInformation();
    void reloadMasterArmClicked();
    void camera1ControlOptionChanged(int formatIndex);
    void camera2ControlOptionChanged(int formatIndex);
    void camera3ControlOptionChanged(int formatIndex);
    void camera4ControlOptionChanged(int formatIndex);
    void camera5ControlOptionChanged(int formatIndex);
    void cameraControlOptionChanged(int camera, int formatIndex);
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
