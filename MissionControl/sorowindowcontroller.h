#ifndef SOROWINDOWCONTROLLER_H
#define SOROWINDOWCONTROLLER_H

#include <QtCore>
#include <QObject>

#include "channel.h"
#include "logger.h"
#include "iniparser.h"
#include "soro_global.h"
#include "armmessage.h"
#include "drivemessage.h"
#include "gimbalmessage.h"
#include "mbedchannel.h"
#include "armglfwmap.h"
#include "driveglfwmap.h"
#include "gimbalglfwmap.h"
#include "glfwmapdialog.h"
#include "latlng.h"
#include "soroini.h"
#include "mcini.h"

#define NO_CONTROLLER -1

namespace Soro {
namespace MissionControl {

class SoroWindowController : public QObject
{
    Q_OBJECT
public:

    explicit SoroWindowController(QObject *presenter = 0);

    ~SoroWindowController();

    const Channel *getControlChannel() const;
    const Channel *getSharedChannel() const;
    const MbedChannel* getMasterArmChannel() const;

private:
    char _buffer[512];
    bool _glfwInitialized = false;
    Logger *_log = NULL;
    char _currentKey = '\0';
    int _initTimerId = TIMER_INACTIVE;

    //used to load configuration options
    SoroIniConfig _soroIniConfig;
    MissionControlIniConfig _mcIniConfig;

    //internet communication channels
    Channel *_controlChannel = NULL;
    Channel *_sharedChannel = NULL;
    QList<Channel*> _sharedChannelNodes;

    //for joystick control
    ArmGlfwMap *_armInputMap = NULL;
    DriveGlfwMap *_driveInputMap = NULL;
    GimbalGlfwMap *_gimbalInputMap = NULL;
    int _controllerId = NO_CONTROLLER;
    int _controlSendTimerId = TIMER_INACTIVE;
    int _inputSelectorTimerId = TIMER_INACTIVE;

    //Arm specific stuff
    MbedChannel *_masterArmChannel = NULL;
    ArmMessage::MasterRanges _masterArmRanges;

    void loadMasterArmConfig();
    void initForGLFW(GlfwMap *map);
    int firstGlfwControllerId();
    GlfwMap* getInputMap();

signals:
    void initialized(const SoroIniConfig& soroConfig,
                     const MissionControlIniConfig& mcConfig);
    void gamepadChanged(QString name);
    void error(QString description);
    void warning(QString description);
    void connectionQualityUpdate(int sharedRtt, int tcpLag);

private slots:
    void sharedChannelMessageReceived(const QByteArray& message);
    void sharedChannelNodeMessageReceived(const QByteArray& message);
    void masterArmMessageReceived(const char *message, int size);
    void sharedChannelStatsUpdate(int rtt, quint64 messagesUp, quint64 messagesDown,
                            int rateUp, int rateDown);

public slots:
    void settingsClicked();
    void currentKeyChanged(char key);

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
