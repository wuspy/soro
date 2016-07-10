#ifndef SORO_CONFIGURATION_H
#define SORO_CONFIGURATION_H

#include <QtCore>

#include "iniparser.h"
#include "logger.h"

namespace Soro {

/* The main configuration loader and model for both mission control and the rover.
 */
struct Configuration {

    bool isLoaded = false;

    QHostAddress ServerAddress;
    Logger::Level LogLevel;
    int MainComputerCameraCount, SecondaryComputerCameraCount;
    quint16 ArmChannelPort, DriveChannelPort, GimbalChannelPort, SharedChannelPort, SecondaryComputerPort;
    quint16 FirstVideoPort;
    quint16 ArmMbedPort, DriveCameraMbedPort;
    quint16 McBroadcastPort, MasterArmPort;
    quint16 AudioStreamPort;
    quint16 RoverGpsServerPort;
    QList<QString> BlacklistedUsbCameras;

    /* Loads the configuration from the default path, and returns true if successful.
     * If there is an error, err will contain a summary of what went wrong.
     */
    bool load(QString file, QString *err);

    /* Sets the max level of the root logger based on what was specified in the loaded configuration
     */
    void applyLogLevel();
};

}

#endif // SORO_CONFIGURATION_H
