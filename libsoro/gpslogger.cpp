#include "gpslogger.h"
#include "logger.h"

#define LOG_TAG "GpsLogger"

namespace Soro {

GpsLogger::GpsLogger(QObject *parent) : QObject(parent)
{

}

bool GpsLogger::startLog(QString file, QDateTime loggedStartTime) {
    if (_fileStream != NULL) {
        delete _fileStream;
        _fileStream = NULL;
    }
    if (_file != NULL) {
        if (_file->isOpen()) _file->close();
        delete _file;
    }
    _file = new QFile(file, this);
    if (_file->exists()) {
        LOG_E(LOG_TAG, "File \'" + file + "\' already exists, I will not overwrite it");
    }
    else if (_file->open(QIODevice::WriteOnly)) {
        _logStartTime = loggedStartTime.toMSecsSinceEpoch();
        _fileStream = new QDataStream(_file);
        *_fileStream << _logStartTime;
        LOG_I(LOG_TAG, "Beginning log with time " + QString::number(_logStartTime));
        return true;
    }
    // could not open the file
    LOG_E(LOG_TAG, "Unable to open the specified logfile for write access (" + file + ")");
    return false;
}

void GpsLogger::stopLog() {
    if (_file) {
        if (_fileStream) {
            delete _fileStream;
            _fileStream = NULL;
            LOG_I(LOG_TAG, "Ending log with start time " + QString::number(_logStartTime));
        }
        if (_file->isOpen()) {
            _file->close();
        }
        delete _file;
        _file = NULL;
    }
}

void GpsLogger::addLocation(NmeaMessage location) {
    if (_fileStream) {
        *_fileStream << (quint32)(QDateTime::currentDateTime().toMSecsSinceEpoch() - _logStartTime)
                    << location;
    }
}

} // namespace Soro
