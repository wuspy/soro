#ifndef COMMONINI_H
#define COMMONINI_H

#include <QtCore>

#include "iniparser.h"

#define SOROINI_PATH "config/soro.ini"
#define SOROINI_TAG_SERVER_ADDRESS "serveraddress"
#define SOROINI_TAG_ARM_SERVER_PORT "armchannelserverport"
#define SOROINI_TAG_DRIVE_SERVER_PORT "drivechannelserverport"
#define SOROINI_TAG_GIMBAL_SERVER_PORT "gimbalchannelserverport"
#define SOROINI_TAG_SHARED_SERVER_PORT "sharedchannelserverport"
#define SOROINI_TAG_VIDEO_SERVER_ADDRESS "videoserveraddress"
#define SOROINI_TAG_ARM_VIDEO_PORT "armvideoport"
#define SOROINI_TAG_DRIVE_VIDEO_PORT "drivevideoport"
#define SOROINI_TAG_GIMBAL_VIDEO_PORT "gimbalvideoport"

namespace Soro {

struct SoroIniConfig {

    QHostAddress serverAddress;
    QHostAddress videoServerAddress;
    quint16 armChannelPort, driveChannelPort, gimbalChannelPort, sharedChannelPort;
    quint16 armVideoPort, driveVideoPort, gimbalVideoPort;

    bool parse(QString *err) {
        QString appPath = QCoreApplication::applicationDirPath();
        IniParser configParser;
        QFile configFile(appPath + "/" + SOROINI_PATH);
        if (!configParser.load(configFile)) {
            *err = "The configuration file " + appPath + "/" + SOROINI_PATH + " is missing or invalid";
            return false;
        }

        int tmp;
        if (!configParser.valueAsIP(SOROINI_TAG_SERVER_ADDRESS, &serverAddress, true)) {
            *err = "No server address found in configuration file";
            return false;
        }
        if (!configParser.valueAsIP(SOROINI_TAG_VIDEO_SERVER_ADDRESS, &videoServerAddress, true)) {
            //assume video server address is same as main server address
            videoServerAddress = serverAddress;
        }
        if (!configParser.valueAsInt(SOROINI_TAG_ARM_SERVER_PORT, &tmp)) {
            *err = "No arm channel port found in configuration file";
            return false;
        }
        armChannelPort = tmp;
        if (!configParser.valueAsInt(SOROINI_TAG_DRIVE_SERVER_PORT, &tmp)) {
            *err = "No drive channel port found in configuration file";
            return false;
        }
        driveChannelPort = tmp;
        if (!configParser.valueAsInt(SOROINI_TAG_GIMBAL_SERVER_PORT, &tmp)) {
            *err = "No gimbal channel port found in configuration file";
            return false;
        }
        gimbalChannelPort = tmp;
        if (!configParser.valueAsInt(SOROINI_TAG_ARM_VIDEO_PORT, &tmp)) {
            *err = "No arm video port found in configuration file";
            return false;
        }
        armVideoPort = tmp;
        if (!configParser.valueAsInt(SOROINI_TAG_DRIVE_VIDEO_PORT, &tmp)) {
            *err = "No drive video  port found in configuration file";
            return false;
        }
        driveVideoPort = tmp;
        if (!configParser.valueAsInt(SOROINI_TAG_GIMBAL_VIDEO_PORT, &tmp)) {
            *err = "No gimbal video  port found in configuration file";
            return false;
        }
        gimbalVideoPort = tmp;
        return true;
    }
};

}

#endif // COMMONINI_H
