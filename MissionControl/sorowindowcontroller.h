#ifndef SOROWINDOWCONTROLLER_H
#define SOROWINDOWCONTROLLER_H

#include <QtCore>
#include <QObject>

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
#include "mcini.h"

namespace Soro {
namespace MissionControl {

class SoroWindowController : public QObject
{
    Q_OBJECT
public:
    enum DriveGamepadMode {
        SingleStick, DualStick
    };

    explicit SoroWindowController(QObject *presenter = 0);

    ~SoroWindowController();

    const Channel *getControlChannel() const;
    const Channel *getSharedChannel() const;
    const MbedChannel* arm_getMasterArmChannel() const;
    const SoroIniLoader *getSoroIniConfig() const;
    const MissionControlIniLoader *getMissionControlIniConfig() const;
    SDL_GameController *getGamepad();
    void drive_setMiddleSkidSteerFactor(float factor);
    void drive_setGamepadMode(DriveGamepadMode mode);
    float drive_getMiddleSkidSteerFactor() const;
    SoroWindowController::DriveGamepadMode drive_getGamepadMode() const;

private:
    char _buffer[512];
    bool _sdlInitialized = false;
    DriveGamepadMode _driveGamepadMode = DualStick;
    float _driveMiddleSkidSteerFactor = 0.4;
    Logger *_log = NULL;

    //used to load configuration options
    SoroIniLoader _soroIniConfig;
    MissionControlIniLoader _mcIniConfig;

    //internet communication channels
    Channel *_controlChannel = NULL;
    Channel *_sharedChannel = NULL;
    QList<Channel*> _sharedChannelNodes;

    //for joystick control
    SDL_GameController *_gameController = NULL;
    int _controlSendTimerId = TIMER_INACTIVE;
    int _inputSelectorTimerId = TIMER_INACTIVE;

    //Arm specific stuff
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

private slots:
    void sharedChannelMessageReceived(const char *message, Channel::MessageSize size);
    void sharedChannelNodeMessageReceived(const char *message, Channel::MessageSize size);
    void arm_masterArmMessageReceived(const char *message, int size);
    void sharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                            int rateUp, int rateDown);

public slots:
    void init();

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
