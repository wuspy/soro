#include "abstractdatarecorder.h"
#include "logger.h"

namespace Soro {

AbstractDataRecorder::AbstractDataRecorder(QString logTag, QObject *parent) : QObject(parent) {
    _logTag = logTag;
}

AbstractDataRecorder::~AbstractDataRecorder() {
    stopLog();
}

bool AbstractDataRecorder::startLog(QString file, QDateTime loggedStartTime) {
    stopLog();
    _file = new QFile(file, this);
    if (_file->exists()) {
        LOG_E(_logTag, "File \'" + file + "\' already exists, I will not overwrite it");
    }
    else if (_file->open(QIODevice::WriteOnly)) {
        _logStartTime = loggedStartTime.toMSecsSinceEpoch();
        _fileStream = new QDataStream(_file);
        *_fileStream << _logStartTime;
        LOG_I(_logTag, "Beginning log with time " + QString::number(_logStartTime));
        _isRecording = true;
        return true;
    }
    // could not open the file
    LOG_E(_logTag, "Unable to open the specified logfile for write access (" + file + ")");
    delete _file;
    _file = NULL;
    return false;
}

void AbstractDataRecorder::stopLog() {
    if (_isRecording) {
        delete _fileStream;
        _fileStream = NULL;
        LOG_I(_logTag, "Ending log with start time " + QString::number(_logStartTime));

        if (_file->isOpen()) {
            _file->close();
        }
        delete _file;
        _file = NULL;
    }
}

void AbstractDataRecorder::recordData(QByteArray data) {
    if (_isRecording) {
        *_fileStream << (quint32)(QDateTime::currentDateTime().toMSecsSinceEpoch() - _logStartTime)
                     << data;
    }
}

} // namespace Soro
