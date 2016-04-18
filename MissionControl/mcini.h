#ifndef MCINI_H
#define MCINI_H

#include <QtCore>

#include "iniparser.h"
#include "logger.h"
#include "socketaddress.h"

#define MISSION_CONTROL_INI_PATH QCoreApplication::applicationDirPath() + "/config/mission_control.ini"

namespace Soro {
namespace MissionControl {

/* Struct to contain and parse the configuration file that is common
 * to mission control and the rover (stuff like the server IP, which side should
 * act as server, ports, etc)
 */
struct MissionControlIniLoader {

    enum InputMode {
        Gamepad, MasterArm
    };

    enum LayoutMode {
        ArmLayoutMode, DriveLayoutMode, GimbalLayoutMode, SpectatorLayoutMode
    };

    QHostAddress CommHostAddress;
    QHostAddress VideoHostAddress;
    QHostAddress NodeHostAddress;
    InputMode ControlInputMode;
    LayoutMode Layout;
    bool MasterNode;
    quint16 *NodePorts = NULL;
    quint16 MasterArmPort;
    SocketAddress MasterNodeAddress;

    ~MissionControlIniLoader();

    /* Loads the configuration from the default path, and returns true if successful.
     * If there is an error, err will contain a summary of what went wrong.
     */
    bool load(QString *err);
};

}
}

#endif // MCINI_H
