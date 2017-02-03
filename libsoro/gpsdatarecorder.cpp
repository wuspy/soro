#include "gpsdatarecorder.h"
#include "logger.h"

#define LOG_TAG "GpsDataRecorder"

namespace Soro {

GpsDataRecorder::GpsDataRecorder(QObject *parent) : AbstractDataRecorder(LOG_TAG, parent) { }

void GpsDataRecorder::addLocation(NmeaMessage location) {
    QByteArray data; QDataStream stream(data);
    stream << location;
    recordData(data);
}

} // namespace Soro
