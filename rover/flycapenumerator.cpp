#include "flycapenumerator.h"

#define LOG_TAG "FlycapEnumerator"

namespace Soro {
namespace Rover {

int FlycapEnumerator::loadCameras() {
    FlyCapture2::Error error;
    FlyCapture2::BusManager busMgr;
    unsigned int numCameras;
    FlyCapture2::PGRGuid guid;
    unsigned int serial;

    _cameras.clear();

    error = busMgr.RescanBus();
    if (error != FlyCapture2::PGRERROR_OK) {
        return 0;
    }

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != FlyCapture2::PGRERROR_OK) {
        return 0;
    }

    for (unsigned int i=0; i < numCameras; i++) {
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != FlyCapture2::PGRERROR_OK) {
            continue;
        }
        error = busMgr.GetCameraSerialNumberFromIndex(i, &serial);
        if (error != FlyCapture2::PGRERROR_OK) {
            continue;
        }
        _cameras.insert(serial, guid);
    }

    return numCameras;
}

QList<FlyCapture2::PGRGuid> FlycapEnumerator::listByGuid() {
    return _cameras.values();
}

bool FlycapEnumerator::cameraExists(unsigned int serial) {
    return _cameras.contains(serial);
}

FlyCapture2::PGRGuid FlycapEnumerator::getGUIDForSerial(unsigned int serial) {
    if (!cameraExists(serial)) {
        return FlyCapture2::PGRGuid();
    }
    return _cameras[serial];
}

} // namespace Rover
} // namespace Soro
