#include "gpsdatarecorder.h"
#include "logger.h"

#define LOG_TAG "GpsDataRecorder"

namespace Soro {

GpsDataRecorder::GpsDataRecorder(QObject *parent) : AbstractDataRecorder(LOG_TAG, "gps", parent) { }

void GpsDataRecorder::addLocation(NmeaMessage location) {
    if (_fileStream) {
        addTimestamp();
        *_fileStream << location;
    }
}

} // namespace Soro
