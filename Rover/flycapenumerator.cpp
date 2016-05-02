#include "flycapenumerator.h"

#define LOG_TAG "FlycapEnumerator"

namespace Soro {
namespace Rover {

int FlycapEnumerator::loadCameras(Logger *log) {
    FlyCapture2::Error error;
    FlyCapture2::BusManager busMgr;
    unsigned int numCameras;
    FlyCapture2::PGRGuid guid;
    unsigned int serial;

    _cameras.clear();

    error = busMgr.RescanBus();
    if (error != FlyCapture2::PGRERROR_OK) {
        if (log != NULL) log->e(LOG_TAG, QString("Cannot scan flycap camera bus: ") + error.GetDescription());
        return -1;
    }

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != FlyCapture2::PGRERROR_OK) {
        if (log != NULL) log->e(LOG_TAG, QString("Cannot query number of cameras: ") + error.GetDescription());
        return -1;
    }
    if (log != NULL) log->i(LOG_TAG, QString("Number of cameras detected: " + QString::number(numCameras)));

    for (unsigned int i=0; i < numCameras; i++) {
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != FlyCapture2::PGRERROR_OK) {
            if (log != NULL) log->e(LOG_TAG, QString("Error getting guid for camera at index " + QString::number(i) + ", skipping..."));
            continue;
        }
        error = busMgr.GetCameraSerialNumberFromIndex(i, &serial);
        if (error != FlyCapture2::PGRERROR_OK) {
            if (log != NULL) log->e(LOG_TAG, QString("Error getting serial for camera at index " + QString::number(i) + ", skipping..."));
            continue;
        }
        if (log != NULL) log->i(LOG_TAG, QString("Found camera " + QString::number(serial) + ", guid=*-" + QString::number(guid.value[3])));
        _cameras.insert(serial, guid);
    }

    return numCameras;
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
