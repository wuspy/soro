/*
 * Copyright 2017 The University of Oklahoma.
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

#include "mbeddataparser.h"

#define LOG_TAG "MbedDataParser"

const char *TAGS[] =
{
    "!", // Wheeldata 1
    "@", // Wheeldata 2
    "#", // Wheeldata 3
    "$", // Wheeldata 4
    "%", // Wheeldata 5
    "^", // Wheeldata 6
    "+!", // IMUdata 1 X
    "+@", // IMUdata 1 Y
    "+#", // IMUdata 1 Z
    "~!", // IMUdata 2 X
    "~@", // IMUdata 2 Y
    "~#" // IMUdata 2 Z
};

namespace Soro {
namespace Rover {

MbedDataParser::MbedDataParser(MbedChannel *mbed, QObject *parent) : QObject(parent) {
    _mbed = mbed;
    connect(_mbed, SIGNAL(messageReceived(MbedChannel*,const char*,int)),
            this, SLOT(messageReceived(MbedChannel*,const char*,int)));
}

MbedDataParser::~MbedDataParser() {
    stopLog();
}

bool MbedDataParser::startLog(QString file, QDateTime loggedStartTime) {
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
        return true;
    }
    // could not open the file
    LOG_E(LOG_TAG, "Unable to open the specified logfile for write access (" + file + ")");
    return false;
}

void MbedDataParser::stopLog() {
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

void MbedDataParser::messageReceived(MbedChannel *mbed, const char* data, int len) {
    _buffer.append(data, len);
}

void MbedDataParser::parseBuffer() {
    if (_buffer.length() == 0) return;

    for (int i = 0; i < 12; i++) {
        if (_buffer.startsWith(TAGS[i])) {
            parseNext(reinterpret_cast<DataTag&>(i), strlen(TAGS[i]));
            parseBuffer();
            return;
        }
    }

    // Unknown start token in buffer, remove chars until one is recognized or
    // the buffer is empty
    LOG_E(LOG_TAG, "Invalid token, buffer contents: " + QString(_buffer));
    _buffer.remove(0, 1);
    parseBuffer();
}

void MbedDataParser::parseNext(DataTag tag, int start) {
    if (start >= _buffer.length()) return;
    int end = start;
    while (QChar(_buffer.at(end)).isNumber()) {
        end++;
        if (end == _buffer.length()) return;
    }

    float value = _buffer.mid(start, end - start).toFloat();
    _buffer.remove(0, end);

    // Append this data to the logfile;
    if (_fileStream) {
        *_fileStream
                << (quint32)(QDateTime::currentDateTime().toMSecsSinceEpoch() - _logStartTime)
                << reinterpret_cast<quint32&>(tag)
                << value;
    }

    emit newData(tag, value);
}

}
}
