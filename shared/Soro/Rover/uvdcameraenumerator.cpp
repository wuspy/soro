#include "uvdcameraenumerator.h"

#define LOG_TAG "UvdCameraEnumerator"
#define _log log

namespace Soro {
namespace Rover {

int UvdCameraEnumerator::loadCameras() {
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

const QList<QString>& UvdCameraEnumerator::listByDeviceName() {
    return _cameras;
}


} // namespace Rover
} // namespace Soro
