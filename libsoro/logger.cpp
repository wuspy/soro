/*
 * Copyright 2016 The University of Oklahoma.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger.h"

namespace Soro {

Logger* Logger::_root = new Logger();

Logger::Logger(QObject *parent) : QObject(parent) {
    // create default text formatting
    _qtLoggerFormat << "[E]\t%1\t%2:\t%3";
    _qtLoggerFormat << "[W]\t%1\t%2:\t%3";
    _qtLoggerFormat << "[I]\t%1\t%2:\t%3";
    _qtLoggerFormat << "[D]\t%1\t%2:\t%3";

    // default unless later set otherwise
    _textFormat << _qtLoggerFormat;
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
        _fileStream = new QTextStream(_file);
        return true;
    }
    // could not open the file
    e("LOGGER", "Unable to open the specified logfile for write access (" + file + ")");
    return false;
}

void Logger::publish(Level level, QString tag, QString message) { //PRIVATE
    // check for file output
    if (level <= _maxFileLevel) {
        QString formatted = _textFormat[reinterpret_cast<unsigned int&>(level) - 1]
                .arg(QTime::currentTime().toString(), tag, message);
        if (_fileStream != NULL) {
            *_fileStream << formatted << endl;
            _fileStream->flush();
        }
    }
    // check for Qt logger output
    if (level <= _maxQtLogLevel) {
        const char *formatted = _qtLoggerFormat[reinterpret_cast<unsigned int&>(level) - 1]
                .arg(QTime::currentTime().toString(), tag, message)
                .toUtf8().constData();
        switch (level) {
        case LogLevelDebug:
            qDebug() << formatted;
        case LogLevelInformation: //no equivalent qInfo() exists
        case LogLevelWarning:
            qWarning() << formatted;
            break;
        case LogLevelError:
            qCritical() << formatted;
            break;
        default:
            break;
        }
    }
}

void Logger::closeLogfile() {
    if (_file) {
        if (_fileStream) {
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

void Logger::setMaxFileLevel(Logger::Level maxLevel) {
    _maxFileLevel = maxLevel;
}

void Logger::setMaxQtLoggerLevel(Logger::Level maxLevel) {
    _maxQtLogLevel = maxLevel;
}

void Logger::setOutputFileTextFormat(const QStringList& format) {
    if (format.size() != 4) {
        e("LOGGER", "Attempted to set invalid text formatting");
    }
    else {
        _textFormat = format;
    }
}

Logger::~Logger() {
    closeLogfile();
}

}
