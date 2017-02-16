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
    width: height
    opacity: 0.8

    onLatencyChanged: {
        if (latency < 0) {
            latencyText.text = "Delay<br><b>N/A</b>"
        }
        else {
            latencyText.text = "Delay<br><b>" + latency.toString() + "ms</b>"
        }
    }


    function clamp(ratio) {
        if (ratio < 0) return 0
        if (ratio > 1) return 1
        return ratio
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

            xCanvas.requestPaint()
            yCanvas.requestPaint()
        }
    }

    transform: Scale { xScale: halfWidth ? 0.5 : 1 }

    HudBackground {
        id: xBackground
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        width:  parent.height / 4
    }

    HudBackground {
        id: yBackground
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        height: parent.height / 4
    }

    HudBackground {
        id: latencyBackground
        backdrop.color: "#4CAF50"
        backdrop.radius: 0
        opacity: 1
        width: height
        height: parent.height / 4
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Text {
            id: latencyText
            anchors.centerIn: parent
            font.pointSize: parent.width / 8
            text: "Delay<br><b>11352ms</b>"
            horizontalAlignment: Text.AlignHCenter
            color: "white"
        }
    }

    Canvas {
        id: xCanvas
        anchors.fill: xBackground
        anchors.topMargin: xBackground.margin + width / 10;
        anchors.leftMargin: anchors.topMargin
        anchors.rightMargin: anchors.topMargin
        anchors.bottomMargin: latencyBackground.height + width / 10

        onPaint: {
            // Get drawing context
            var context = getContext("2d");
            context.fillStyle = "#ffffff"
            context.clearRect(0, 0, width, height)

            var blobSize = width / 5

            // Draw bottom blob
            var startBlobX = (width / 2) + ((width / 2 - blobSize / 2) * (xHistory[xHistory.length - 1]))
            context.beginPath()
            context.ellipse(startBlobX - (blobSize / 2),
                            height - blobSize,
                            blobSize,
                            blobSize)
            context.fill()

            // Draw top blob
            var index = Math.floor(xHistory.length - 1 - (latency / refreshInterval))
            var endBlobX = -1;
            if (index >= 0 && index < xHistory.length) {
                endBlobX = (width / 2) + ((width / 2 - blobSize / 2) * (xHistory[index]))
                context.beginPath()
                context.ellipse(endBlobX - (blobSize / 2),
                                0,
                                blobSize,
                                blobSize)
                context.fill()
            }

            // Draw path connecting
            context.strokeStyle = "#88ffffff"
            context.lineWidth = blobSize / 5
            context.beginPath()
            context.moveTo(startBlobX, height - (blobSize / 2))
            var resolution = height / linePixelsPerStep
            for (var i = 1; i < resolution; i++) {
                index = Math.floor(xHistory.length - 1 - ((latency / refreshInterval) * (i / resolution)))
                if (index >= xHistory.length) return
                context.lineTo((width / 2) + ((width / 2 - blobSize / 2) * (xHistory[index])),
                               (height - blobSize) - ((height - 2 * blobSize) * (i / resolution)))
            }

            if (endBlobX > 0) {
                context.lineTo(endBlobX, blobSize / 2);
            }

            context.stroke()
        }
    }

    Canvas {
        id: yCanvas
        anchors.fill: yBackground
        anchors.topMargin: yBackground.margin + height / 10;
        anchors.bottomMargin: anchors.topMargin
        anchors.rightMargin: anchors.topMargin
        anchors.leftMargin: latencyBackground.width + height / 10

        onPaint: {
            // Get drawing context
            var context = getContext("2d");
            context.fillStyle = "#ffffff"
            context.clearRect(0, 0, width, height)

            var blobSize = height / 5

            // Draw bottom blob
            var startBlobY = (height / 2) + ((height / 2 - blobSize / 2) * (yHistory[yHistory.length - 1]))
            context.beginPath()
            context.ellipse(0,
                            startBlobY - (blobSize / 2),
                            blobSize,
                            blobSize)
            context.fill()

            // Draw top blob
            var index = Math.floor(xHistory.length - 1 - (latency / refreshInterval))
            var endBlobY = -1;
            if (index >= 0 && index < xHistory.length) {
                endBlobY = (height / 2) + ((height / 2 - blobSize / 2) * (yHistory[index]))
                context.beginPath()
                context.ellipse(width - blobSize,
                                endBlobY - (blobSize / 2),
                                blobSize,
                                blobSize)
                context.fill()
            }

            // Draw path connecting
            context.strokeStyle = "#88ffffff"
            context.lineWidth = blobSize / 5
            context.beginPath()
            context.moveTo(blobSize / 2, startBlobY)
            var resolution = width / linePixelsPerStep
            for (var i = 1; i < resolution; i++) {
                index = Math.floor(yHistory.length - 1 - ((latency / refreshInterval) * (i / resolution)))
                if (index >= yHistory.length) return
                context.lineTo(blobSize + ((width - 2 * blobSize) * (i / resolution)),
                               (height / 2) + ((height / 2 - blobSize / 2) * (yHistory[index])))
            }

            if (endBlobY > 0) {
                context.lineTo(width - blobSize / 2, endBlobY);
            }

            context.stroke()
        }
    }
}
