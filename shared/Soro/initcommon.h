#ifndef INITCOMMON_H
#define INITCOMMON_H

#include "soro_global.h"
#include "logger.h"
#include "configuration.h"

#define ENV_SORO_INI_PATH "SORO_INI_PATH"

namespace Soro {

static bool findConfigAndLoad(Configuration *config) {
    QString error;
    if (config->load(QCoreApplication::applicationDirPath() + "/config/soro.ini", &error)) {
        LOG_I("init", "Loaded configuration from ./config/soro.ini");
        return true;
    }
    else {
        LOG_E("init", "Failed to load configuration from ./config/soro.ini (" + error + ") trying something else");
        if (config->load(QCoreApplication::applicationDirPath() + "/soro.ini", &error)) {
            LOG_I("init", "Loaded configuration from ./soro.ini");
            return true;
        }
        else {
            LOG_E("init", "Failed to load configuration from ./soro.ini (" + error + ") nowhere else to look");
        }
    }
    return false;
}

static bool init(Configuration *config, QString logFilePrefix) {
    // set root log output file
    Logger::rootLogger()->setLogfile(QCoreApplication::applicationDirPath()
                                     + "/" + logFilePrefix + "_" + QDateTime::currentDateTime().toString("M-dd_h.mm_AP") + ".log");
    Logger::rootLogger()->setMaxQtLoggerLevel(Logger::LogLevelInformation);

    LOG_I("init", "-------------------------------------------------------");
    LOG_I("init", "-------------------------------------------------------");
    LOG_I("init", "-------------------------------------------------------");
    LOG_I("init", "Starting up...");
    LOG_I("init", "-------------------------------------------------------");
    LOG_I("init", "-------------------------------------------------------");
    LOG_I("init", "-------------------------------------------------------");

    // load configuration file
    QProcessEnvironment env;
    QString error;
    QString path = env.value(ENV_SORO_INI_PATH, "");

    if (path.isEmpty()) {
        // no config path specified in the enviornment vars
        if (!findConfigAndLoad(config)) return false;
    }
    else {
        LOG_I("init", "Loading configuration from enviornment specified path " + path);
        if (!config->load(path, &error)) {
            LOG_E("init", "Failed to load enviornment specified configuration file (" + error + ") retrying with default path");
            if (!findConfigAndLoad(config)) return false;
        }
    }

    return true;
}

}

#endif // INITCOMMON_H
