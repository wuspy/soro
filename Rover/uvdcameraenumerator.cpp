#include "uvdcameraenumerator.h"

#define LOG_TAG "UvdCameraEnumerator"
#define _log log

namespace Soro {
namespace Rover {

int UvdCameraEnumerator::loadCameras() {
    _cameras.clear();
    int total = 0;
#ifdef __linux__
    QDirIterator dev("/dev");
    while (dev.hasNext()) {
        QString file = dev.next();
        if (file.contains("video")) {
            _cameras.append(file);
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
