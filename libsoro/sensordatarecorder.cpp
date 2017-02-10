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

#include "sensordatarecorder.h"

#define LOG_TAG "SensorDataRecorder"

namespace Soro {

SensorDataRecorder::SensorDataRecorder(QObject *parent)
    : AbstractDataRecorder(LOG_TAG, "sensor", parent) {
}

void SensorDataRecorder::newData(const char* data, int len) {
    _buffer.append(data, len);

    parseBuffer(true);
}

void SensorDataRecorder::parseBuffer(bool logErrors) {
    if (_buffer.length() < 4) return; // 4  is the size of a complete data point
    char tag = _buffer.at(0);

    if (isValidTag(tag)) {
        bool ok;
        int data = QString(_buffer.mid(1, 3)).toFloat(&ok);
        if (ok) {
            // Append this data to the logfile;
            if (_fileStream) {
                addTimestamp();
                *_fileStream << tag << data;
            }
            emit dataParsed(tag, data);
            // Try to parse more data
            _buffer.remove(0, 4);
            parseBuffer(true);
            return;
        }
    }

    // Something went wrong trying to parse
    if (logErrors) {
        if (_fileStream) {
            addTimestamp();
            *_fileStream << DATATAG_ERROR;
        }
        emit parseError();
    }
    // Remove the first char and try to parse again
    _buffer.remove(0, 1);
    parseBuffer(false);
}

bool SensorDataRecorder::isValidTag(char c) {
    return (c == DATATAG_WHEELDATA_A) ||
            (c == DATATAG_WHEELDATA_B) ||
            (c == DATATAG_WHEELDATA_C) ||
            (c == DATATAG_WHEELDATA_D) ||
            (c == DATATAG_WHEELDATA_E) ||
            (c == DATATAG_WHEELDATA_F) ||
            (c == DATATAG_IMUDATA_1_X) ||
            (c == DATATAG_IMUDATA_1_X) ||
            (c == DATATAG_IMUDATA_1_Y) ||
            (c == DATATAG_IMUDATA_1_Z) ||
            (c == DATATAG_IMUDATA_2_X) ||
            (c == DATATAG_IMUDATA_2_Y) ||
            (c == DATATAG_IMUDATA_2_Z);
}

} // namespace Soro
