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

#include "csvrecorder.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDir>

#define LOG_TAG "CsvRecorder"

namespace Soro {

QVariant CsvDataSeries::getValue() const {
    return _value;
}

qint64 CsvDataSeries::getValueAge() const {
    return QDateTime::currentDateTime().toMSecsSinceEpoch() - _valueTime;
}

void CsvDataSeries::update(QVariant value) {
    _value = value;
    _valueTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

////////////////////////////////////////

CsvRecorder::CsvRecorder(QObject *parent) : QObject(parent) {
    _updateInterval = 100;
}

bool CsvRecorder::startLog(QDateTime loggedStartTime) {
    stopLog();

    QString filePath = QCoreApplication::applicationDirPath() + "/../research_data";

     if (!QDir(filePath).exists()) {
        LOG_I(LOG_TAG, filePath + " directory does not exist, creating it");
        if (!QDir().mkpath(filePath)) {
            LOG_E(LOG_TAG, "Cannot create " + filePath + " directory, data cannot be logged");
            return false;
        }
    }

    _logStartTime = loggedStartTime.toMSecsSinceEpoch();
    filePath += "/" + QString::number(_logStartTime) + ".csv";
    _file = new QFile(filePath, this);

    if (_file->exists()) {
        LOG_W(LOG_TAG, "File \'" + filePath + "\' already exists, overwriting it");
    }
    if (_file->open(QIODevice::WriteOnly)) {
        _fileStream = new QTextStream(_file);
        // Write header to file
        *_fileStream << "Recording started at " << loggedStartTime.toString() << "\n";
        *_fileStream << "Rows in this file were updated every " << _updateInterval << " milleseconds\n";
        *_fileStream << "\n";

        foreach (const CsvDataSeries* column, _columns) {
            *_fileStream << column->getSeriesName() << "," << column->getSeriesName() << " (age),";
        }
        *_fileStream << "\n";

        LOG_I(LOG_TAG, "Starting log " + QString::number(_logStartTime));
        START_TIMER(_updateTimerId, _updateInterval);
        _isRecording = true;
        emit logStarted(loggedStartTime);
        return true;
    }
    // could not open the file
    LOG_E(LOG_TAG, "Unable to open the specified logfile for write access (" + filePath + ")");
    delete _file;
    _file = nullptr;
    _logStartTime = 0;
    return false;
}

void CsvRecorder::stopLog() {
    if (_isRecording) {
        KILL_TIMER(_updateTimerId);
        delete _fileStream;
        _fileStream = nullptr;
        LOG_I(LOG_TAG, "Ending log " + QString::number(_logStartTime));

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

int CsvRecorder::getUpdateInterval() const {
    return _updateInterval;
}

const QList<const CsvDataSeries*>& CsvRecorder::getColumns() const {
    return _columns;
}

void CsvRecorder::setUpdateInterval(int interval) {
    if (_isRecording) {
        LOG_E(LOG_TAG, "Cannot change update interval while recording");
        return;
    }
    _updateInterval = interval;
}

void CsvRecorder::addColumn(const CsvDataSeries *series) {
    if (_isRecording) {
        LOG_E(LOG_TAG, "Cannot modify column array while recording");
        return;
    }
    if (!_columns.contains(series)) {
        _columns.append(series);
    }
}

void CsvRecorder::removeColumn(const CsvDataSeries *series) {
    if (_isRecording) {
        LOG_E(LOG_TAG, "Cannot modify column array while recording");
        return;
    }
    if (_columns.contains(series)) {
        _columns.removeAll(series);
    }
}

void CsvRecorder::clearColumns() {
    if (_isRecording) {
        LOG_E(LOG_TAG, "Cannot modify column array while recording");
        return;
    }
    _columns.clear();
}

void CsvRecorder::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);

    if ((e->timerId() == _updateTimerId) && _fileStream) {
        foreach (const CsvDataSeries *column, _columns) {
            *_fileStream << column->getValue().toString() << "," << column->getValueAge() << ",";
        }
        *_fileStream << "\n";
    }
}

bool CsvRecorder::isRecording() const {
    return _isRecording;
}

qint64 CsvRecorder::getStartTime() const {
    return _logStartTime;
}

} // namespace Soro
