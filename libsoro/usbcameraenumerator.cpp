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

#include "usbcameraenumerator.h"
#include "logger.h"

#define LOG_TAG "UvdCameraEnumerator"
#define _log log

namespace Soro {

int UsbCameraEnumerator::loadCameras() {
    _cameras.clear();
    int total = 0;
#ifdef __linux__
    QDir dev("/dev");
    QStringList allFiles = dev.entryList(QDir::NoDotAndDotDot
                                     | QDir::System
                                     | QDir::Hidden
                                     | QDir::AllDirs
                                     | QDir::Files, QDir::DirsFirst);
    foreach (QString file, allFiles) {
        if (file.contains("video")) {
            _cameras.append("/dev/" + file);
            total++;
        }
    }
#endif
    return total;
}

const QList<QString>& UsbCameraEnumerator::listByDeviceName() {
    return _cameras;
}

} // namespace Soro
