#include "soroini.h"

const char *_tag_ServerAddress = "ServerAddress";
const char *_tag_ServerSide = "ServerSide";
const char *_tag_ArmServerPort = "ArmChannelPort";
const char *_tag_DriveServerPort = "DriveChannelPort";
const char *_tag_GimbalServerPort = "GimbalChannelPort";
const char *_tag_SharedServerPort = "SharedChannelPort";
const char *_tag_VideoServerAddress = "VideoServerAddress";
const char *_tag_ArmVideoPort = "ArmVideoPort";
const char *_tag_DriveVideoPort = "DriveVideoPort";
const char *_tag_GimbalVideoPort = "GimbalVideoPort";
const char *_tag_LogLevel = "LogLevel";
const char *_value_RoverIsServer = "Rover";
const char *_value_MissionControlIsServer = "MissionControl";
const char *_value_LogLevelDebug = "Debug";
const char *_value_LogLevelInfo = "Information";
const char *_value_LogLevelWarn = "Warning";
const char *_value_LogLevelError = "Error";
const char *_value_LogDisabled = "Disabled";
const char *_tag_ArmMbedPort = "ArmMbedPort";
const char *_tag_DriveMbedPort = "DriveMbedPort";
const char *_tag_GimbalMbedPort = "GimbalMbedPort";
const char *_tag_MasterArmPort = "MasterArmPort";
const char *_tag_McSubnetBroadcastPort = "McSubnetBroadcastPort";

namespace Soro {

bool SoroIniLoader::load(QString *err) {
    IniParser configParser;
    QFile configFile(SOROINI_PATH);
    if (!configParser.load(configFile)) {
        *err = "The configuration file " + SOROINI_PATH + " is missing or invalid";
        return false;
    }

    int tmp;
    if (!configParser.valueAsIP(_tag_ServerAddress, &ServerAddress, true)) {
        *err = "No server address found in configuration file";
        return false;
    }
    QString serverSide = configParser.value(_tag_ServerSide);
    if (QString::compare(serverSide, _value_RoverIsServer, Qt::CaseInsensitive) == 0) {
        ServerSide = RoverEndPoint;
        //we cannot know were to send the video when we act as the server.
        //wait for mission control to connect first, then send it to that address.
        VideoServerAddress = QHostAddress::Null;
    }
    else {
        //default to mission control as server
        ServerSide = MissionControlEndPoint;
        if (!configParser.valueAsIP(_tag_VideoServerAddress, &VideoServerAddress, true)) {
            //assume video server address is same as main server address
            VideoServerAddress = ServerAddress;
        }
    }
    QString logLevel = configParser.value(_tag_LogLevel);
    if (QString::compare(logLevel, _value_LogLevelDebug, Qt::CaseInsensitive) == 0) {
        LogLevel = LOG_LEVEL_DEBUG;
    }
    else if (QString::compare(logLevel, _value_LogLevelWarn, Qt::CaseInsensitive) == 0) {
        LogLevel = LOG_LEVEL_WARN;
    }
    else if (QString::compare(logLevel, _value_LogLevelError, Qt::CaseInsensitive) == 0) {
        LogLevel = LOG_LEVEL_ERROR;
    }
    else if (QString::compare(logLevel, _value_LogDisabled, Qt::CaseInsensitive) == 0) {
        LogLevel = LOG_LEVEL_DISABLED;
    }
    else {
        //default to info level
        LogLevel = LOG_LEVEL_INFORMATION;
    }
    if (!configParser.valueAsInt(_tag_ArmServerPort, &tmp)) {
        *err = "No arm channel port found in configuration file";
        return false;
    }
    ArmChannelPort = tmp;
    if (!configParser.valueAsInt(_tag_DriveServerPort, &tmp)) {
        *err = "No drive channel port found in configuration file";
        return false;
    }
    DriveChannelPort = tmp;
    if (!configParser.valueAsInt(_tag_GimbalServerPort, &tmp)) {
        *err = "No gimbal channel port found in configuration file";
        return false;
    }
    GimbalChannelPort = tmp;
    if (!configParser.valueAsInt(_tag_SharedServerPort, &tmp)) {
        *err = "No shared channel port found in configuration file";
        return false;
    }
    SharedChannelPort = tmp;
    if (!configParser.valueAsInt(_tag_ArmVideoPort, &tmp)) {
        *err = "No arm video port found in configuration file";
        return false;
    }
    ArmVideoPort = tmp;
    if (!configParser.valueAsInt(_tag_DriveVideoPort, &tmp)) {
        *err = "No drive video port found in configuration file";
        return false;
    }
    DriveVideoPort = tmp;
    if (!configParser.valueAsInt(_tag_GimbalVideoPort, &tmp)) {
        *err = "No gimbal video port found in configuration file";
        return false;
    }
    GimbalVideoPort = tmp;
    if (!configParser.valueAsInt(_tag_ArmMbedPort, &tmp)) {
        *err = "No arm mbed port found in configuration file";
        return false;
    }
    ArmMbedPort = tmp;
    if (!configParser.valueAsInt(_tag_DriveMbedPort, &tmp)) {
        *err = "No drive mbed port found in configuration file";
        return false;
    }
    DriveMbedPort = tmp;
    if (!configParser.valueAsInt(_tag_GimbalMbedPort, &tmp)) {
        *err = "No gimbal mbed port found in configuration file";
        return false;
    }
    GimbalMbedPort = tmp;
    if (!configParser.valueAsInt(_tag_McSubnetBroadcastPort, &tmp)) {
        *err = "No mission control subnet broadcast port found in configuration file";
        return false;
    }
    McBroadcastPort = tmp;
    if (!configParser.valueAsInt(_tag_MasterArmPort, &tmp)) {
        *err = "No master arm port found in configuration file";
        return false;
    }
    MasterArmPort = tmp;
    return true;
}

void SoroIniLoader::applyLogLevel(Logger*& log) {
    if (log == NULL) return;
    switch (LogLevel) {
    case LOG_LEVEL_DISABLED:
        log->i("SoroIniConfig", "The configuration file specifies to disable logging, goodbye!");
        delete log;
        log = NULL;
        break;
    case LOG_LEVEL_DEBUG:
        log->i("SoroIniConfig", "The maximum log level for this file is DEBUG");
        log->MaxLevel = LOG_LEVEL_DEBUG;
        break;
    case LOG_LEVEL_INFORMATION:
        log->i("SoroIniConfig", "The maximum log level for this file is INFORMATION");
        log->MaxLevel = LOG_LEVEL_INFORMATION;
        break;
    case LOG_LEVEL_WARN:
        log->i("SoroIniConfig", "The maximum log level for this file is WARNING");
        log->MaxLevel = LOG_LEVEL_WARN;
        break;
    case LOG_LEVEL_ERROR:
        log->i("SoroIniConfig", "The maximum log level for this file is ERROR");
        log->MaxLevel = LOG_LEVEL_ERROR;
        break;
    }
}

}
