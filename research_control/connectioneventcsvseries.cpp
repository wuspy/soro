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

#include "connectioneventcsvseries.h"

namespace Soro {
namespace MissionControl {

ConnectionEventCsvSeries::ConnectionEventCsvSeries(const Channel* driveChannel, const Channel* sharedChannel, QObject *parent)
        : QObject(parent) {

    connect(driveChannel, &Channel::stateChanged, this, &ConnectionEventCsvSeries::driveChannelStateChanged);
    connect(sharedChannel, &Channel::stateChanged, this, &ConnectionEventCsvSeries::sharedChannelStateChanged);
}

QString ConnectionEventCsvSeries::getSeriesName() const {
    return "Events";
}

bool ConnectionEventCsvSeries::shouldKeepOldValues() const {
    return false;
}

void ConnectionEventCsvSeries::driveChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        update(QVariant("Drive Channel CONNECTED"));
        break;
    default:
        update(QVariant("Drive Channel DISCONNECTED"));
        break;
    }
}

void ConnectionEventCsvSeries::sharedChannelStateChanged(Channel::State state) {
    switch (state) {
    case Channel::ConnectedState:
        update(QVariant("Shared Channel CONNECTED"));
        break;
    default:
        update(QVariant("Shared Channel DISCONNECTED"));
        break;
    }
}

} // namespace MissionControl
} // namespace Soro
