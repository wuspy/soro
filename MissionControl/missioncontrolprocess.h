#ifndef SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H
#define SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H

#include <QtCore>
#include <QMainWindow>
#include <QObject>
#include <QUdpSocket>

#include "channel.h"
#include "logger.h"
#include "iniparser.h"
#include "soro_global.h"
#include "armmessage.h"
#include "drivemessage.h"
#include "gimbalmessage.h"
#include "mbedchannel.h"
#include "latlng.h"
#include "masterarmconfig.h"
#include "configuration.h"
#include "camerawidget.h"
#include "videoclient.h"
#include "audioclient.h"
#include "audioplayer.h"
#include "mainwindow.h"
#include "armcontrolsystem.h"
#include "drivecontrolsystem.h"
#include "cameracontrolsystem.h"
#include "missioncontrolnetwork.h"

namespace Soro {
namespace MissionControl {

class MissionControlProcess : public QObject {
    Q_OBJECT
public:

    explicit MissionControlProcess(const Configuration *config, GamepadManager *gamepad,
                                   MissionControlNetwork *mcNetwork, ControlSystem *controlSystem, QObject *parent = 0);

    ~MissionControlProcess();

    const Configuration* getConfiguration() const;

signals:
    void windowClosed();

private:
    //the main UI components

    MainWindow *_ui;

    bool _roverSharedChannelConnected = false;
    bool _ignoreGamepadVideoButtons = false;

    // Used to load configuration options
    const Configuration *_config;

    // Communicates with the rover if the mission control is configured as broker
    Channel *_roverChannel = NULL;

    // Coordinates networking amoung mission controls
    MissionControlNetwork *_mcNetwork = NULL;

    // Connects to a controllable rover system
    ControlSystem *_controlSystem = NULL;

    GamepadManager *_gamepad = NULL;

    // Timers
    int _rttStatTimerId = TIMER_INACTIVE;
    int _droppedPacketTimerId = TIMER_INACTIVE;
    int _bitrateUpdateTimerId = TIMER_INACTIVE;

    // These hold the video clients when in master configuration
    QList<VideoClient*> _videoClients; // camera ID is by index
    QList<VideoFormat> _videoFormats; // camera ID is by index
    QMap<int, CameraWidget*> _assignedCameraWidgets; // camera ID is by key
    QList<QString> _cameraNames; // camera ID is by index
    QList<CameraWidget*> _freeCameraWidgets;

    // GPS message cache stored by the broker mission control
    QList<NmeaMessage*> _gpsMessages;

    // Audio stream subsystem
    AudioClient *_audioClient = NULL;
    AudioPlayer *_audioPlayer = NULL;
    AudioFormat _audioFormat = AudioFormat_Null;

    // cache of last status information
    RoverSubsystemState _lastArmSubsystemState = UnknownSubsystemState;
    RoverSubsystemState _lastDriveGimbalSubsystemState = UnknownSubsystemState;
    RoverSubsystemState _lastSecondaryComputerSubsystemState = UnknownSubsystemState;

    void handleSharedChannelMessage(const char *message, Channel::MessageSize size);
    void handleCameraStateChange(int cameraID, VideoClient::State state, VideoFormat encoding, QString errorString);
    void handleAudioStateChanged(AudioClient::State state, AudioFormat encoding, QString errorString);
    void handleRoverSharedChannelStateChanged(Channel::State state);
    void handleCameraNameChanged(int camera, QString newName);
    void playStreamOnWidget(int cameraID, CameraWidget *widget, VideoFormat format);
    void endStreamOnWidget(CameraWidget *widget, QString reason);
    void playAudio();

private slots:
    void onNewMissionControlClient(Channel *channel);
    void roverSharedChannelStateChanged(Channel *channel, Channel::State state);
    void roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void videoClientStateChanged(MediaClient *client, MediaClient::State state);
    void audioClientStateChanged(MediaClient *client, MediaClient::State state);
    void cycleVideosClockwise();
    void cycleVideosCounterClockwise();
    void cameraFormatSelected(int camera, VideoFormat format);
    void audioStreamFormatSelected(AudioFormat format);
    void audioStreamMuteSelected(bool mute);
    void cameraNameEdited(int camera, QString newName);

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SORO_MISSIONCONTROL_SOROWINDOWCONTROLLER_H
