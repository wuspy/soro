/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "roverconfigloader.h"

namespace Soro {
namespace Rover {

bool RoverConfigLoader::load(QString *error) {
    IniParser configParser;
    QFile configFile(QCoreApplication::applicationDirPath() + "/soro_rover_config.ini");
    if (!configParser.load(configFile)) {
        *error = "The configuration file soro_rover_config.ini is missing or invalid";
        return false;
    }

    int tmp;
    if (!configParser.valueAsInt("Computer1CameraCount", &tmp)) {
        *error = "No Computer1CameraCount entry found in configuration file soro_rover_config.ini";
        return false;
    }
    _computer1CameraCount = tmp;

    if (!configParser.valueAsInt("Computer2CameraCount", &tmp)) {
        *error = "No Computer2CameraCount entry found in configuration file soro_rover_config.ini";
        return false;
    }
    _computer2CameraCount = tmp;

    _blacklistCameras = configParser.valueAsStringList("CameraBlacklist");

    return true;
}

int RoverConfigLoader::getComputer1CameraCount() {
    return _computer1CameraCount;
}

int RoverConfigLoader::getComputer2CameraCount() {
    return _computer2CameraCount;
}

QList<QString> RoverConfigLoader::getBlacklistedCameras() {
    return _blacklistCameras;
}

} // namespace Rover
} // namespace Soro
