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
#include "serialchannel3.h"
#include "armglfwmap.h"
#include "driveglfwmap.h"
#include "gimbalglfwmap.h"
#include "glfwmapdialog.h"
#include "videowindow.h"
#include "latlng.h"
#include "commonini.h"

#define NO_CONTROLLER -1

namespace Soro {
namespace MissionControl {

class SoroWindowController : public QObject
{
    Q_OBJECT
public:
    enum InputMode {
        GLFW, MasterArm
    };

    enum LayoutMode {
        ArmLayoutMode, DriveLayoutMode, GimbalLayoutMode, SpectatorLayoutMode
    };

    explicit SoroWindowController(QObject *presenter = 0);

    ~SoroWindowController();

    const Channel *getControlChannel() const;
    const Channel *getSharedChannel() const;
    const SerialChannel3* getMasterArmSerial() const;

private:
    InputMode _inputMode = GLFW;
    LayoutMode _mode = SpectatorLayoutMode;
    char _buffer[512];
    bool _glfwInitialized = false;
    Logger *_log = NULL;
    char _currentKey = '\0';
    int _initTimerId = TIMER_INACTIVE;

    //shared configuration
    SoroIniConfig _soroIniConfig;

    //internet communication channels
    Channel *_controlChannel = NULL;
    Channel *_sharedChannel = NULL;

    //for joystick control
    ArmGlfwMap *_armInputMap = NULL;
    DriveGlfwMap *_driveInputMap = NULL;
    GimbalGlfwMap *_gimbalInputMap = NULL;
    int _controllerId = NO_CONTROLLER;
    int _controlSendTimerId = TIMER_INACTIVE;
    int _inputSelectorTimerId = TIMER_INACTIVE;

    //Arm specific stuff
    SerialChannel3 *_masterArmSerial = NULL;
    ArmMessage::MasterRanges _masterArmRanges;

    void loadMasterArmConfig();
    void initForGLFW(GlfwMap *map);
    int firstGlfwControllerId();
    GlfwMap* getInputMap();

signals:
    void initialized(const SoroIniConfig& config,
                     SoroWindowController::LayoutMode mode,
                     SoroWindowController::InputMode inputMode);
    void gamepadChanged(QString name);
    void error(QString description);
    void warning(QString description);

private slots:
    void sharedChannelMessageReceived(const QByteArray& message);
    void masterArmSerialMessageReceived(const char *message, int size);

public slots:
    void settingsClicked();
    void currentKeyChanged(char key);

protected:
    void timerEvent(QTimerEvent *e);

};

}
}

#endif // SOROWINDOWCONTROLLER_H
