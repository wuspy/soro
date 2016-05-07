#include "logger.h"

namespace Soro {

const QString Logger::_levelFormatters[4] = {
    "[E]\t%1\t%2:\t%3",
    "[W]\t%1\t%2:\t%3",
    "[I]\t%1\t%2:\t%3",
    "[D]\t%1\t%2:\t%3"
};

const QString Logger::_levelFormattersHTML[4] = {
    "<div style=\"color:#b71c1c\">%1&emsp;E/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#0d47a1\">%1&emsp;W/<i>%2</i>:&emsp;%3</div>",
    "<div>%1&emsp;I/<i>%2</i>:&emsp;%3</div>",
    "<div style=\"color:#dddddd\">%1&emsp;D/<i>%2</i>:&emsp;%3</div>"
};

Logger::Logger(QObject *parent) : QObject(parent) {
  MaxLevel = LOG_LEVEL_DEBUG;
  MaxQtLoggerLevel = LOG_LEVEL_INFORMATION;
  RouteToQtLogger = false;
}

bool Logger::setLogfile(QString file) {
    if (_fileStream != NULL) {
        _fileStream->flush();
        delete _fileStream;
        _fileStream = NULL;
    }
    if (_file != NULL) {
        if (_file->isOpen()) _file->close();
        delete _file;
    }
    _file = new QFile(file, this);
    if (_file->open(QIODevice::Append)) {
        qDebug() << "Logger: Set log file" << file;
        _fileStream = new QTextStream(_file);
        return true;
    }
    //could not open the file
    qCritical() << "Logger: Unable to open the specified logfile for write access (" << file << ")";
    return false;
}

void Logger::publish(int level, QString tag, QString message) { //PRIVATE
    if (level <= MaxLevel) {
        emit logMessagePublished(level, tag, message);
        QString formatted = _levelFormatters[level].arg(QTime::currentTime().toString(), tag, message);
        emit logMessagePublished(formatted);
        if (_fileStream != NULL) {
            *_fileStream << formatted << endl;
        }
        if ((RouteToQtLogger) & (level <= MaxQtLoggerLevel)) {
            switch (level) {
            case LOG_LEVEL_DEBUG:
                qDebug() << formatted;
            case LOG_LEVEL_INFORMATION: //no equivilent qInfo() exists
            case LOG_LEVEL_WARN:
                qWarning() << formatted;
                break;
            case LOG_LEVEL_ERROR:
                qCritical() << formatted;
                break;
            }
        }
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

}
