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

#include "armmessage.h"
#include "channel.h"
#include "soro_global.h"
#include "mbedchannel.h"
#include "nmeamessage.h"
#include "camerawidget.h"
#include "camerawindow.h"
#include "videocontrolwidget.h"

namespace Ui {
    class SoroMainWindow;
}

namespace Soro {
namespace MissionControl {

class SoroMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SoroMainWindow(QWidget *parent = 0);
    ~SoroMainWindow();

    CameraWidget* getTopCameraWidget();
    CameraWidget* getBottomCameraWidget();
    CameraWidget* getFullscreenCameraWidget();

    bool isMuteAudioSelected();


private:
    Ui::SoroMainWindow *ui;
    CameraWindow *_videoWindow;

    bool _fullscreen = false;
    QMovie *_preloaderMovie;
    static const QString _logLevelFormattersHTML[4];
    QString _lastName = "Unnamed";
    bool _lastIsMaster = false;
    Role _lastRole;
    Channel::State _lastControlChannelState;
    Channel::State _lastSharedChannelState;
    Channel::State _lastMccChannelState;
    RoverSubsystemState _lastArmSubsystemState;
    RoverSubsystemState _lastDriveCameraSubsystemState;
    RoverSubsystemState _lastSecondaryComputerState;
    int _lastDroppedPacketPercent = 0;
    int _lastRtt = 0;

    int _clearGpsStatusTimerId = TIMER_INACTIVE;

    QMessageBox *_messageBoxHolder = NULL;

signals:
    void settingsClicked();
    void chatMessageEntered(QString message);
    void cycleVideosClockwise();
    void cycleVideosCounterclockwise();
    void cameraFormatChanged(int camera, VideoFormat format);
    void audioStreamFormatChanged(AudioFormat format);
    void audioStreamMuteChanged(bool mute);
    void cameraNameEdited(int camera, QString name);
    void reloadMasterArmClicked();

public slots:
    void onFatalError(QString description);
    void onWarning(QString description);
    void onGamepadChanged(SDL_GameController *controller);
    void onLocationUpdate(const NmeaMessage& location);
    void onControlChannelStateChanged(Channel::State state);
    void onMccChannelStateChanged(Channel::State state);
    void onSharedChannelStateChanged(Channel::State state);
    void onRttUpdate(int rtt);
    void onBitrateUpdate(quint64 bpsRoverDown, quint64 bpsRoverUp);
    void onDroppedPacketRateUpdate(int droppedRatePercent);
    void onArmSubsystemStateChanged(RoverSubsystemState state);
    void onDriveCameraSubsystemStateChanged(RoverSubsystemState state);
    void onSecondaryComputerStateChanged(RoverSubsystemState state);
    void arm_onMasterArmStateChanged(MbedChannel::State state);
    void arm_onMasterArmUpdate(const char *armMessage);
    void onNotification(NotificationType type, QString sender, QString message);
    void onRoleChanged(Role role);
    void onNameChanged(QString name);
    void onMasterChanged(bool isMaster);
    void onCameraFormatChanged(int camera, VideoFormat format);
    void onAudioFormatChanged(AudioFormat format);
    void setCameraName(int camera, QString name);

private slots:
    void updateStatusBar();
    void updateConnectionStateInformation();
    void updateSubsystemStateInformation();
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
};

}
}

#endif // SOROMAINWINDOW_H
