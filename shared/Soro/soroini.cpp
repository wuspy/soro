#include "soroini.h"

const char *_tag_ServerAddress = "ServerAddress";
const char *_tag_ArmServerPort = "ArmChannelPort";
const char *_tag_DriveServerPort = "DriveChannelPort";
const char *_tag_GimbalServerPort = "GimbalChannelPort";
const char *_tag_SharedServerPort = "SharedChannelPort";
const char *_tag_UvdCameraBlacklist = "UVDCameraBlacklist";
const char *_tag_FirstVideoPort = "FirstVideoStreamPort";
const char *_tag_MainComputerCameraCount = "MainComputerCameraCount";
const char *_tag_SecondaryComputerCameraCount = "SecondaryComputerCameraCount";
const char *_tag_LogLevel = "LogLevel";
const char *_value_RoverIsServer = "Rover";
const char *_value_MissionControlIsServer = "MissionControl";
const char *_value_LogLevelDebug = "Debug";
const char *_value_LogLevelInfo = "Information";
const char *_value_LogLevelWarn = "Warning";
const char *_value_LogLevelError = "Error";
const char *_value_LogDisabled = "Disabled";
const char *_tag_ArmMbedPort = "ArmMbedPort";
const char *_tag_DriveCameraMbedPort = "DriveCameraMbedPort";
const char *_tag_SecondaryComputerPort = "SecondaryComputerPort";
const char *_tag_MasterArmPort = "MasterArmPort";
const char *_tag_McSubnetBroadcastPort = "McSubnetBroadcastPort";
const char *_tag_AudioStreamPort = "AudioStreamPort";
const char *_tag_RoverGpsServerPort = "RoverGpsServerPort";

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
    if (!configParser.valueAsInt(_tag_ArmMbedPort, &tmp)) {
        *err = "No arm mbed port found in configuration file";
        return false;
    }
    ArmMbedPort = tmp;
    if (!configParser.valueAsInt(_tag_DriveCameraMbedPort, &tmp)) {
        *err = "No drive/camera mbed port found in configuration file";
        return false;
    }
    DriveCameraMbedPort = tmp;
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
    if (!configParser.valueAsInt(_tag_FirstVideoPort, &tmp)) {
        *err = "No first video stream port found in configuration file";
        return false;
    }
    FirstVideoPort = tmp;
    if (!configParser.valueAsInt(_tag_SecondaryComputerPort, &tmp)) {
        *err = "No secondary computer port found in configuration file";
        return false;
    }
    SecondaryComputerPort = tmp;
    if (!configParser.valueAsInt(_tag_AudioStreamPort, &tmp)) {
        *err = "No audio stream port found in configuration file";
        return false;
    }
    AudioStreamPort = tmp;
    if (!configParser.valueAsInt(_tag_RoverGpsServerPort, &tmp)) {
        *err = "No rover GPS server port found in configuration file";
        return false;
    }
    RoverGpsServerPort = tmp;
    BlacklistedUvdCameras = configParser.valueAsStringList(_tag_UvdCameraBlacklist);
    if (!configParser.valueAsInt(_tag_MainComputerCameraCount, &MainComputerCameraCount)) {
        *err = "No main computer camera count found in configuration file";
        return false;
    }
    if (!configParser.valueAsInt(_tag_SecondaryComputerCameraCount, &SecondaryComputerCameraCount)) {
        *err = "No secondary computer camera count found in configuration file";
        return false;
    }
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
