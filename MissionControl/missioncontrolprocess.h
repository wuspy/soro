#ifndef SOROWINDOWCONTROLLER_H
#define SOROWINDOWCONTROLLER_H

#include <QtCore>
#include <QMainWindow>
#include <QObject>
#include <QUdpSocket>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

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
#include "soroini.h"
#include "camerawidget.h"
#include "videoclient.h"
#include "audioclient.h"
#include "audioplayer.h"
#include "soromainwindow.h"

namespace Soro {
namespace MissionControl {

class MissionControlProcess : public QObject {
    Q_OBJECT
public:

    explicit MissionControlProcess(QString name, bool masterSubnetNode, Role role, QObject *parent = 0);

    ~MissionControlProcess();

    QString getName() const;
    const SoroIniLoader *getConfiguration() const;
    void drive_setMiddleSkidSteerFactor(float factor);
    void drive_setGamepadMode(DriveGamepadMode mode);
    float drive_getMiddleSkidSteerFactor() const;
    DriveGamepadMode drive_getGamepadMode() const;
    Role getRole() const;
    bool isMasterSubnetNode() const;
    Logger *getLogger();

private:
    // Used as scratch space for reading gamepad data, master arm data,
    // subnet broadcasts, etc
    char _buffer[512];

    //the main UI
    SoroMainWindow *ui;

    // Holds the unique name of this mission control that the user can choose (except Bill)
    QString _name;

    // General configuration
    bool _isMaster;
    Role _role;
    Logger *_log = NULL;

    bool _roverSharedChannelConnected = false;
    bool _ignoreGamepadVideoButtons = false;

    // Used to connect to other mission control computers on the same subnet
    QUdpSocket *_broadcastSocket = NULL;

    // Used to load configuration options
    SoroIniLoader _config;

    // Internet communication channels
    Channel *_controlChannel = NULL;
    Channel *_sharedChannel = NULL;

    // used by the master mission control to relay data
    // from the shared channel to other mission controls
    // and vice versa
    QList<Channel*> _slaveMissionControlChannels;
    QList<Role> _slaveMissionControlRoles;
    QList<Channel*> _newSlaveMissionControls;

    // SDL joystick control stuff
    bool _sdlInitialized = false;
    float _driveMiddleSkidSteerFactor = 0.2; //lower is faster, higher is slower
    DriveGamepadMode _driveGamepadMode = DualStickDrive;
    SDL_GameController *_gameController = NULL;
    int _controlSendTimerId = TIMER_INACTIVE;
    int _inputSelectorTimerId = TIMER_INACTIVE;
    int _broadcastSharedChannelInfoTimerId = TIMER_INACTIVE;
    int _masterResponseWatchdogTimerId = TIMER_INACTIVE;
    int _rttStatTimerId = TIMER_INACTIVE;
    int _droppedPacketTimerId = TIMER_INACTIVE;
    int _bitrateUpdateTimerId = TIMER_INACTIVE;

    // These hold the video clients when in master configuration
    QList<VideoClient*> _videoClients; // camera ID is by index
    QList<VideoFormat> _videoFormats; // camera ID is by index
    QMap<int, CameraWidget*> _assignedCameraWidgets; // camera ID is by key
    QList<QString> _cameraNames; // camera ID is by index
    QList<CameraWidget*> _freeCameraWidgets;

    // Audio stream subsystem
    AudioClient *_audioClient;
    AudioPlayer *_audioPlayer;

    // Master arm stuff
    MbedChannel *_masterArmChannel = NULL;
    MasterArmConfig _masterArmRanges;

    // cache of last status information
    RoverSubsystemState _lastArmSubsystemState = UnknownSubsystemState;
    RoverSubsystemState _lastDriveGimbalSubsystemState = UnknownSubsystemState;
    RoverSubsystemState _lastSecondaryComputerSubsystemState = UnknownSubsystemState;

    void arm_loadMasterArmConfig();
    void initSDL();
    void quitSDL();
    void handleSharedChannelMessage(const char *message, Channel::MessageSize size);
    void broadcastSharedMessage(const char *message, int size, bool includeRover, Channel *exclude = 0);
    void handleCameraStateChange(int cameraID, VideoClient::State state, VideoFormat encoding, QString errorString);
    void handleAudioStateChanged(AudioClient::State state, AudioFormat encoding, QString errorString);
    void handleRoverSharedChannelStateChanged(Channel::State state);
    void handleCameraNameChanged(int camera, QString newName);
    void playStreamOnWidget(int cameraID, CameraWidget *widget, VideoFormat format);
    void endStreamOnWidget(CameraWidget *widget, QString reason);

private slots:
    void slave_masterSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void slave_masterSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_roverSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_slaveSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void master_slaveSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void arm_masterArmMessageReceived(const char *message, int size);
    void arm_masterArmStateChanged(MbedChannel *channel, MbedChannel::State state);
    void master_broadcastSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);
    void videoClientStateChanged(MediaClient *client, MediaClient::State state);
    void audioClientStateChanged(MediaClient *client, MediaClient::State state);
    void init();
    void chatMessageEntered(QString message);

    void cycleVideosClockwise();
    void cycleVideosCounterClockwise();
    void cameraFormatSelected(int camera, VideoFormat format);
    void audioStreamFormatSelected(AudioFormat format);
    void audioStreamMuteSelected(bool mute);
    void cameraNameEdited(int camera, QString newName);
    void sendWelcomePackets();

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
