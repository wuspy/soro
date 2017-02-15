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
    property int refreshInterval: 5
    property int linePixelsPerStep: 1

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

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: background.margin + width / 10;

        function clamp(ratio) {
            if (ratio < 0) return 0
            if (ratio > 1) return 1
            return ratio
        }

        onPaint: {

            latency = 1000
            // Get drawing context
            var context = getContext("2d");
            context.fillStyle = "#ffffff"
            context.clearRect(0, 0, width, height)

            var blobSize = width / 5

            // Draw bottom blob
            var bottomBlobX = (width / 2) + ((width / 2 - blobSize / 2) * (xHistory[xHistory.length - 1]))
            context.beginPath()
            context.ellipse(bottomBlobX - (blobSize / 2),
                            height - blobSize,
                            blobSize,
                            blobSize)
            context.fill()

            // Draw top blob
            var index = Math.floor(xHistory.length - 1 - (latency / refreshInterval))
            var topBlobX = -1;
            if (index >= 0 && index < xHistory.length) {
                topBlobX = (width / 2) + ((width / 2 - blobSize / 2) * (xHistory[index]))
                context.beginPath()
                context.ellipse(topBlobX - (blobSize / 2),
                                0,
                                blobSize,
                                blobSize)
                context.fill()
            }

            // Draw path connecting
            context.strokeStyle = "#88ffffff"
            context.lineWidth = blobSize / 5
            context.beginPath()
            context.moveTo(bottomBlobX, height - (blobSize / 2))
            var resolution = height / linePixelsPerStep
            for (var i = 1; i < resolution; i++) {
                index = Math.floor(xHistory.length - 1 - ((latency / refreshInterval) * (i / resolution)))
                if (index >= xHistory.length) return
                context.lineTo((width / 2) + ((width / 2 - blobSize / 2) * (xHistory[index])),
                               (height - blobSize) - ((height - 2 * blobSize) * (i / resolution)))

                var x = (width / 2) + ((width / 2 - blobSize / 2) * (xHistory[index]))
                var y = (height - blobSize) - ((height - 2 * blobSize) * (i / resolution))
            }

            if (topBlobX > 0) {
                context.lineTo(topBlobX, blobSize / 2);
            }

            context.stroke()
        }
    }
}
