#include "abstractdatarecorder.h"
#include "logger.h"

namespace Soro {

AbstractDataRecorder::AbstractDataRecorder(QString logTag, QString logDirectory, QObject *parent) : QObject(parent) {
    _logTag = logTag;
    _logDir = logDirectory;
}

AbstractDataRecorder::~AbstractDataRecorder() {
    stopLog();
}

bool AbstractDataRecorder::startLog(QDateTime loggedStartTime) {
    stopLog();

    QString filePath = QCoreApplication::applicationDirPath() + "/../research_data/" + _logDir;

     if (!QDir(filePath).exists()) {
        LOG_I(_logTag, filePath + " directory does not exist, creating it");
        if (!QDir().mkpath(filePath)) {
            LOG_E(_logTag, "Cannot create " + filePath + " directory, data cannot be logged");
            return false;
        }
    }

    _logStartTime = loggedStartTime.toMSecsSinceEpoch();
    filePath += "/" + QString::number(_logStartTime);
    _file = new QFile(filePath, this);

    if (_file->exists()) {
        LOG_W(_logTag, "File \'" + filePath + "\' already exists, overwriting it");
    }
    if (_file->open(QIODevice::WriteOnly)) {
        _fileStream = new QDataStream(_file);
        _fileStream->setByteOrder(QDataStream::BigEndian);
        *_fileStream << _logStartTime;
        LOG_I(_logTag, "Starting log " + QString::number(_logStartTime));
        _isRecording = true;
        emit logStarted(loggedStartTime);
        return true;
    }
    // could not open the file
    LOG_E(_logTag, "Unable to open the specified logfile for write access (" + filePath + ")");
    delete _file;
    _file = nullptr;
    _logStartTime = 0;
    return false;
}

void AbstractDataRecorder::stopLog() {
    if (_isRecording) {
        delete _fileStream;
        _fileStream = nullptr;
        LOG_I(_logTag, "Ending log " + QString::number(_logStartTime));

        if (_file->isOpen()) {
            _file->close();
        }
        delete _file;
        _file = nullptr;
        _isRecording = false;
        _logStartTime = 0;
        emit logStopped();
    }
}

qint64 AbstractDataRecorder::getStartTime() const {
    return _logStartTime;
}

void AbstractDataRecorder::addTimestamp() {
    if (_fileStream) {
        *_fileStream << (quint32)(QDateTime::currentDateTime().toMSecsSinceEpoch() - _logStartTime);
    }
}

bool AbstractDataRecorder::isRecording() const {
    return _isRecording;
}

} // namespace Soro
