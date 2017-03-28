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

#include "sensordataparser.h"

#define LOG_TAG "SensorDataRecorder"

namespace Soro {

SensorDataParser::SensorDataParser(QObject *parent) : QObject(parent) {

}

void SensorDataParser::newData(const char* data, int len) {
    _buffer.append(data, len);

    parseBuffer(true);
}

void SensorDataParser::parseBuffer(bool logErrors) {
    if (_buffer.length() < 4) return; // 4  is the size of a complete data point
    char tag = _buffer.at(0);

    if (isValidTag(tag)) {
        bool ok;
        int data = QString(_buffer.mid(1, 3)).toFloat(&ok);
        if (ok) {
            _wheelPowerASeries.update(tag, data);
            _wheelPowerBSeries.update(tag, data);
            _wheelPowerCSeries.update(tag, data);
            _wheelPowerDSeries.update(tag, data);
            _wheelPowerESeries.update(tag, data);
            _wheelPowerFSeries.update(tag, data);
            _imuRearYawSeries.update(tag, data);
            _imuRearPitchSeries.update(tag, data);
            _imuRearRollSeries.update(tag, data);
            _imuFrontYawSeries.update(tag, data);
            _imuFrontPitchSeries.update(tag, data);
            _imuFrontRollSeries.update(tag, data);

            emit dataParsed(tag, data);
            // Try to parse more data
            _buffer.remove(0, 4);
            parseBuffer(true);
            return;
        }
    }

    // Something went wrong trying to parse
    // Remove the first char and try to parse again
    _buffer.remove(0, 1);
    parseBuffer(false);
}

const SensorDataParser::WheelPowerASeries* SensorDataParser::getWheelPowerASeries() const {
    return &_wheelPowerASeries;
}

const SensorDataParser::WheelPowerBSeries* SensorDataParser::getWheelPowerBSeries() const {
    return &_wheelPowerBSeries;
}

const SensorDataParser::WheelPowerCSeries* SensorDataParser::getWheelPowerCSeries() const {
    return &_wheelPowerCSeries;
}

const SensorDataParser::WheelPowerDSeries* SensorDataParser::getWheelPowerDSeries() const {
    return &_wheelPowerDSeries;
}

const SensorDataParser::WheelPowerESeries* SensorDataParser::getWheelPowerESeries() const {
    return &_wheelPowerESeries;
}

const SensorDataParser::WheelPowerFSeries* SensorDataParser::getWheelPowerFSeries() const {
    return &_wheelPowerFSeries;
}

const SensorDataParser::ImuRearYawSeries* SensorDataParser::getImuRearYawSeries() const {
    return &_imuRearYawSeries;
}

const SensorDataParser::ImuRearPitchSeries* SensorDataParser::getImuRearPitchSeries() const {
    return &_imuRearPitchSeries;
}

const SensorDataParser::ImuRearRollSeries* SensorDataParser::getImuRearRollSeries() const {
    return &_imuRearRollSeries;
}

const SensorDataParser::ImuFrontYawSeries* SensorDataParser::getImuFrontYawSeries() const {
    return &_imuFrontYawSeries;
}

const SensorDataParser::ImuFrontPitchSeries* SensorDataParser::getImuFrontPitchSeries() const {
    return &_imuFrontPitchSeries;
}

const SensorDataParser::ImuFrontRollSeries* SensorDataParser::getImuFrontRollSeries() const {
    return &_imuFrontRollSeries;
}

bool SensorDataParser::isValidTag(char c) {
    return (c == DATATAG_WHEELPOWER_A) ||
            (c == DATATAG_WHEELPOWER_B) ||
            (c == DATATAG_WHEELPOWER_C) ||
            (c == DATATAG_WHEELPOWER_D) ||
            (c == DATATAG_WHEELPOWER_E) ||
            (c == DATATAG_WHEELPOWER_F) ||
            (c == DATATAG_IMUDATA_REAR_YAW) ||
            (c == DATATAG_IMUDATA_REAR_YAW) ||
            (c == DATATAG_IMUDATA_REAR_PITCH) ||
            (c == DATATAG_IMUDATA_REAR_ROLL) ||
            (c == DATATAG_IMUDATA_FRONT_YAW) ||
            (c == DATATAG_IMUDATA_FRONT_PITCH) ||
            (c == DATATAG_IMUDATA_FRONT_ROLL);
}

} // namespace Soro
