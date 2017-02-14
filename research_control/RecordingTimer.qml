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

import QtQuick 2.0

/*
  Timer to keep track of how long a test has been running.
  */
Timer {
    interval: 1000
    repeat: true

    property Text timeLabel;
    property int elapsed

    onTriggered: {
        elapsed++
        var elapsedHours = Math.floor(elapsedTime / 3600)
        var elapsedMinutes = Math.floor((elapsedTime - (elapsedHours * 3600)) / 60)
        var elapsedSeconds = Math.floor((elapsedTime - (elapsedHours * 3600)) - (elapsedMinutes * 60));

        if (elapsedMinutes.toString().length == 1) {
            elapsedMinutes = "0" + elapsedMinutes
        }
        if (elapsedSeconds.toString().length == 1) {
            elapsedSeconds = "0" + elapsedSeconds
        }

        var timeString = elapsedHours + ":" + elapsedMinutes + ":" + elapsedSeconds

        timeLabel.text = timeString
    }
}
