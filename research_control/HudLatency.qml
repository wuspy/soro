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
    property real xValue: 0
    property real yValue: 0
    property int refreshInterval: 20

    // Should be in the range -1, +1
    property var xHistory: [0, 0, 0]
    property var yHistory: [0, 0, 0]

    height: 600
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

    Timer {
        id: frameTimer
        interval: refreshInterval
        running: true
        repeat: true

        function clamp(a) {
            if (a > 1) return 1
            if (a < -1) return -1
            return a
        }

        onTriggered: {
            // Update history buffer
            if (xHistory.length > 999) xHistory.shift();
            xHistory.push(clamp(xValue));
            if (yHistory.length > 999) yHistory.shift();
            yHistory.push(clamp(yValue));

            canvas.requestPaint()
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

    Canvas {
        id: canvas
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: latencyText.bottom
        anchors.topMargin: width / 10
        anchors.margins: background.margin + width / 10;

        function clamp(ratio) {
            if (ratio < 0) return 0
            if (ratio > 1) return 1
            return ratio
        }

        onPaint: {
            // Get drawing context
            var context = getContext("2d");
            context.fillStyle = "white"
            context.clearRect(0, 0, width, height)

            var blobSize = width / 4

            // Draw bottom blob
            context.beginPath()
            context.ellipse((width / 2) - (blobSize / 2) + ((width / 2 - blobSize / 2) * (xHistory[xHistory.length - 1])),
                            height - blobSize,
                            blobSize,
                            blobSize)
            context.fill()

            // Draw top blob
            var index = xHistory.length - 1 - (latency / refreshInterval);
            if (index >= 0 && index < xHistory.length) {
                context.beginPath()
                context.ellipse((width / 2) - (blobSize / 2) + ((width / 2 - blobSize / 2) * (xHistory[index])),
                                0,
                                blobSize,
                                blobSize)
                context.fill()
            }
        }
    }
}
