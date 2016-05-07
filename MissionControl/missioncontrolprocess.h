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

namespace Soro {
namespace MissionControl {

class MissionControlProcess : public QObject {
    Q_OBJECT
public:
    enum DriveGamepadMode {
        SingleStickDrive, DualStickDrive
    };

    enum Role {
        ArmOperatorRole, DriverRole, CameraOperatorRole, SpectatorRole
    };

    enum NotificationType {
        RoverNotification, MCCNotification, ChatNotification
    };

    explicit MissionControlProcess(QString name, CameraWidget *topVideo, CameraWidget *bottomVideo, CameraWidget *fullscreenVideo,
                                   bool masterSubnetNode, MissionControlProcess::Role role, QMainWindow *presenter = 0);

    ~MissionControlProcess();

    QString getName() const;
    const SoroIniLoader *getConfiguration() const;
    void drive_setMiddleSkidSteerFactor(float factor);
    void drive_setGamepadMode(DriveGamepadMode mode);
    float drive_getMiddleSkidSteerFactor() const;
    MissionControlProcess::DriveGamepadMode drive_getGamepadMode() const;
    MissionControlProcess::Role getRole() const;
    bool isMasterSubnetNode() const;
    void cycleVideosClockwise();
    void cycleVideosCounterClockwise();

private:
    // Used as scratch space for reading gamepad data, master arm data,
    // subnet broadcasts, etc
    char _buffer[512];

    // Holds the unique name of this mission control that the user can choose (except Bill)
    QString _name;

    // General configuration
    bool _masterMissionControl;
    MissionControlProcess::Role _role;
    Logger *_log = NULL;

    bool _roverSharedChannelConnected = false;

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

    // The widgets that display video
    CameraWidget *_topVideoWidget;
    CameraWidget *_bottomVideoWidget;
    CameraWidget *_fullscreenVideoWidget;

    // These hold the video clients when in master configuration
    QList<VideoClient*> _videoClients; // camera ID is by index
    QList<StreamFormat> _streamFormats; // camera ID is by index
    QMap<int, CameraWidget*> _videoWidgets; // camera ID is by key

    // Master arm stuff
    MbedChannel *_masterArmChannel = NULL;
    MasterArmConfig _masterArmRanges;

    void arm_loadMasterArmConfig();
    void initSDL();
    void quitSDL();
    void handleSharedChannelMessage(const char *message, Channel::MessageSize size);
    void handleSharedChannelPingUpdate(int ping);
    void broadcastSharedMessage(const char *message, int size, bool includeRover, Channel *exclude = 0);
    void playCamera(int cameraID, CameraWidget *widget);

signals:
    void initializedSDL();
    void fatalError(QString description);
    void warning(QString description);
    void gamepadChanged(SDL_GameController *controller);
    void connectionStateChanged(Channel::State controlChannelState, Channel::State mccNetworkState, Channel::State sharedChannelState);
    void rttUpdate(int rtt);
    void droppedPacketRateUpdate(int droppedRatePercent);
    void roverSystemStateUpdate(RoverSubsystemState armSystemState, RoverSubsystemState driveCameraSystemState,
                                RoverSubsystemState secondaryComputerState);
    void roverCameraUpdate(RoverCameraState camera1State, RoverCameraState camera2State, RoverCameraState camera3State,
                           RoverCameraState camera4State, RoverCameraState camera5State);
    void arm_masterArmStateChanged(MbedChannel *channel, MbedChannel::State state);
    void notification(MissionControlProcess::NotificationType type, QString sender, QString message);

private slots:
    void slave_masterSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void slave_masterSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_roverSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_slaveSharedChannelStateChanged(Channel *channel, Channel::State state);
    void master_roverSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void master_slaveSharedChannelMessageReceived(Channel *channel, const char *message, Channel::MessageSize size);
    void arm_masterArmMessageReceived(const char *message, int size);
    void master_broadcastSocketReadyRead();
    void controlChannelStateChanged(Channel *channel, Channel::State state);

public slots:
    void init();
    void postChatMessage(QString message);

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
