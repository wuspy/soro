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
#include "videostreamwidget.h"
#include "videoclient.h"

namespace Soro {
namespace MissionControl {

class MissionControlProcess : public QObject {
    Q_OBJECT
public:
    enum DriveGamepadMode {
        SingleStick, DualStick
    };

    enum Role {
        ArmOperator, Driver, CameraOperator, Spectator
    };

    explicit MissionControlProcess(VideoStreamWidget *topVideo, VideoStreamWidget *bottomVideo, bool masterSubnetNode, MissionControlProcess::Role role, QMainWindow *presenter = 0);

    ~MissionControlProcess();

    const Channel *getControlChannel() const;
    const Channel *getSharedChannel() const;
    const MbedChannel *arm_getMasterArmChannel() const;
    const SoroIniLoader *getSoroIniConfig() const;
    SDL_GameController *getGamepad();
    void drive_setMiddleSkidSteerFactor(float factor);
    void drive_setGamepadMode(DriveGamepadMode mode);
    float drive_getMiddleSkidSteerFactor() const;
    MissionControlProcess::DriveGamepadMode drive_getGamepadMode() const;
    MissionControlProcess::Role getRole() const;
    bool isMasterSubnetNode() const;

private:
    // Used as scratch space for reading gamepad data, master arm data,
    // subnet broadcasts, etc
    char _buffer[512];

    // General configuration
    bool _masterNode;
    MissionControlProcess::Role _role;
    Logger *_log = NULL;

    // Used to connect to other mission control computers on the same subnet
    QUdpSocket *_broadcastSocket = NULL;

    // Used to load configuration options
    SoroIniLoader _soroIniConfig;

    // Internet communication channels
    Channel *_controlChannel = NULL;
    Channel *_sharedChannel = NULL;
    QList<Channel*> _sharedChannelNodes;

    // SDL joystick control stuff
    bool _sdlInitialized = false;
    float _driveMiddleSkidSteerFactor = 0.2; //lower is faster, higher is slower
    DriveGamepadMode _driveGamepadMode = DualStick;
    SDL_GameController *_gameController = NULL;
    int _controlSendTimerId = TIMER_INACTIVE;
    int _inputSelectorTimerId = TIMER_INACTIVE;
    int _broadcastSharedChannelInfoTimerId = TIMER_INACTIVE;
    int _pruneSharedChannelsTimerId = TIMER_INACTIVE;

    // The widgets that display video
    VideoStreamWidget *_topVideoWidget;
    VideoStreamWidget *_bottomVideoWidget;

    // These hold the video clients when in master configuration
    VideoClient *_armVideoClient = NULL;
    VideoClient *_driveVideoClient = NULL;
    VideoClient *_gimbalVideoClient = NULL;
    VideoClient *_fisheyeVideoClient = NULL;

    // Master arm stuff
    MbedChannel *_masterArmChannel = NULL;
    MasterArmConfig _masterArmRanges;

    void arm_loadMasterArmConfig();
    void initSDL();
    void quitSDL();

signals:
    void initializedSDL();
    void error(QString description);
    void warning(QString description);
    void gamepadChanged(SDL_GameController *controller);
    void connectionQualityUpdate(int sharedRtt, int tcpLag);
    void controlChannelStateChanged(Channel::State state);
    void sharedChannelStateChanged(Channel::State state);
    void controlChannelStatsUpdate(int rtt, quint64 msg_up, quint64 msg_down, int rate_up, int rate_down);
    void arm_masterArmStateChanged(MbedChannel::State state);

private slots:
    void handleSharedChannelMessage(const char *message, Channel::MessageSize size);
    void roverSharedChannelMessageReceived(const char *message, Channel::MessageSize size);
    void nodeSharedChannelMessageReceived(const char *message, Channel::MessageSize size);
    void roverSharedChannelStateChanged(Channel::State state);
    void slaveSharedChannelStateChanged(Channel::State state);
    void arm_masterArmMessageReceived(const char *message, int size);
    void roverSharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                            int rateUp, int rateDown);
    void broadcastSocketReadyRead();

public slots:
    void init();

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
