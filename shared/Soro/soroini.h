#ifndef SOROINI_H
#define SOROINI_H

#include <QtCore>

#include "iniparser.h"
#include "logger.h"

#define SOROINI_PATH QCoreApplication::applicationDirPath() + "/config/soro.ini"

namespace Soro {

/* Struct to contain and parse the configuration file that is common
 * to mission control and the rover (stuff like the server IP, which side should
 * act as server, ports, etc)
 */
struct SoroIniLoader {

    enum EndPoint {
        RoverEndPoint, MissionControlEndPoint
    };

    QHostAddress ServerAddress;
    QHostAddress VideoServerAddress;
    EndPoint ServerSide;
    int LogLevel;
    quint16 ArmChannelPort, DriveChannelPort, GimbalChannelPort, SharedChannelPort;
    quint16 ArmVideoPort, DriveVideoPort, GimbalVideoPort, FisheyeVideoPort;
    quint16 ArmMbedPort, DriveMbedPort, GimbalMbedPort;
    quint16 McBroadcastPort, MasterArmPort;
    QString armCameraDevice, driveCameraDevice, gimbalCameraDevice, fisheyeCameraDevice;

    /* Loads the configuration from the default path, and returns true if successful.
     * If there is an error, err will contain a summary of what went wrong.
     */
    bool load(QString *err);

    /* Configures a logger object based on the log level specified in the
     * config file. If it is set to LOG_LEVEL_DISABLED, this will delete &
     * null the logger!!!
     */
    void applyLogLevel(Logger*& log);
};

}

#endif // SOROINI_H
