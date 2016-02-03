#include "logger.h"

const QString Logger::_levelStrings[3] = { "I", "W", "E" };
const QString Logger::_levelFormatters[3] = {
    "<div>%1&emsp;I/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#0d47a1\">%1&emsp;W/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#b71c1c\">%1&emsp;E/<i>%2</i>:&emsp;%3</div>"
};

const QString Logger::LOG_TAG = "Logger";

Logger::Logger(QObject *parent) : QObject(parent) {
  _file = NULL;
  _fileStream = NULL;
}

bool Logger::setLogfile(QString file) {
    _file = new QFile(file);
    if (_file->open(QIODevice::ReadWrite)) {
        _fileStream = new QTextStream(_file);
        return true;
    }
    //could not open the file
    e(LOG_TAG, "Unable to open the specified logfile for write access (" + file + ")");
    return false;
}

void Logger::i(QString tag, QString message) {
    publish(LEVEL_INFORMATION, tag, message);
}

void Logger::w(QString tag, QString message) {
    publish(LEVEL_WARN, tag, message);
}

void Logger::e(QString tag, QString message) {
    publish(LEVEL_ERROR, tag, message);
}

void Logger::publish(qint32 level, QString tag, QString message) {
    emit onLogMessagePublished(level, tag, message);
    QString formatted = _levelFormatters[level].arg(QTime::currentTime().toString(), tag, message);
    emit onLogMessagePublished(formatted);
    if (_fileStream != NULL) {
        *_fileStream << formatted << "\n";
        _fileStream->flush();
    }
}

void Logger::closeLogfile() {
    if (_file != NULL) {
        if (_fileStream != NULL) {
            delete _fileStream;
            _fileStream = NULL;
        }
        if (_file->isOpen()) {
            _file->close();
        }
        delete _file;
        _file = NULL;
    }
}

Logger::~Logger() {
    closeLogfile();
}
