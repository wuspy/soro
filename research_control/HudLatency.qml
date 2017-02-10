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

import QtQuick 2.4
import QtGraphicalEffects 1.0

Item {
    property bool halfWidth: false
    property int latency: 0
    property int xValue: 0
    property int yValue: 0

    height: 160
    width: height / 3
    opacity: 0.8

    onLatencyChanged: {
        if (latency < 0) {
            latencyText.text = "N/A"
        }
        else {
            latencyText.text = latency.toString() + "ms"
        }
    }

    transform: Scale { xScale: halfWidth ? 0.5 : 1 }

    HudBackground {
        id: background
        anchors.fill: parent
    }

    Text {
        id: latencyText
        color: "white"
        font.pointSize: Math.pow(parent.width, 1.2) / 23
        text: "19999ms"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.margins: background.margin + 10
    }
}
