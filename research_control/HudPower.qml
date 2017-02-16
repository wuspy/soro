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
import QtGraphicalEffects 1.0

Item {
    property bool halfWidth: false
    property int wheelFLPower: 0
    property int wheelFRPower: 0
    property int wheelMLPower: 0
    property int wheelMRPower: 0
    property int wheelBLPower: 0
    property int wheelBRPower: 0

    /* Idle power (wheel will be pure green)
      */
    property int powerBaseline: 150
    /* Power that indicates a very high load (wheel will be pure red)
      */
    property int powerCritical: 500

    onWheelFLPowerChanged: canvas.requestPaint()
    onWheelFRPowerChanged: canvas.requestPaint()
    onWheelMLPowerChanged: canvas.requestPaint()
    onWheelMRPowerChanged: canvas.requestPaint()
    onWheelBLPowerChanged: canvas.requestPaint()
    onWheelBRPowerChanged: canvas.requestPaint()
    onPowerBaselineChanged: canvas.requestPaint()
    onPowerCriticalChanged: canvas.requestPaint()

    width: height * 0.8
    height: 300
    opacity: 0.8

    transform: Scale { xScale: halfWidth ? 0.5 : 1 }

    HudBackground {
        id: background
        anchors.fill: parent
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: background.margin + width / 10;

        property real colorValue: 0.86666
        property real colorSat: 0.89803
        property real colorBaselineHue: 0.26666
        property real colorCriticalHue: 0.00000

        function clamp(ratio) {
            if (ratio < 0) return 0
            if (ratio > 1) return 1
            return ratio
        }

        onPaint: {
            // Get drawing context
            var context = getContext("2d");
            context.strokeStyle = "white"
            context.lineWidth = width / 30

            var hueRange = colorCriticalHue - colorBaselineHue
            var powerRange = powerCritical - powerBaseline

            var flRatio = clamp((wheelFLPower - powerBaseline) / powerRange)
            var frRatio = clamp((wheelFRPower - powerBaseline) / powerRange)
            var mlRatio = clamp((wheelMLPower - powerBaseline) / powerRange)
            var mrRatio = clamp((wheelMRPower - powerBaseline) / powerRange)
            var blRatio = clamp((wheelBLPower - powerBaseline) / powerRange)
            var brRatio = clamp((wheelBRPower - powerBaseline) / powerRange)

            // Draw front axel
            context.beginPath()
            context.moveTo(width * 2 / 5 - context.lineWidth, height / 8)
            context.lineTo(width - width * 2 / 5 + context.lineWidth, height / 8)
            context.stroke()

            // Draw middle axel
            context.beginPath()
            context.moveTo(width * 2 / 5 - context.lineWidth, height / 2)
            context.lineTo(width - width * 2 / 5 + context.lineWidth, height / 2)
            context.stroke()

            // Draw back axel
            context.beginPath()
            context.moveTo(width * 2 / 5 - context.lineWidth, height - height / 8)
            context.lineTo(width - width * 2 / 5 + context.lineWidth, height - height / 8)
            context.stroke()

            // Draw frame
            context.beginPath()
            context.moveTo(width / 2, height / 8)
            context.lineTo(width / 2, height - height / 8)
            context.stroke()

            // Draw FL wheel
            context.beginPath()
            context.fillStyle = wheelFLPower > 0 ? Qt.hsva(colorBaselineHue + flRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(0, 0);
            context.lineTo(width / 5, 0)
            context.lineTo(width * 2 / 5, height / 8)
            context.lineTo(width / 5, height / 4)
            context.lineTo(0, height / 4)
            context.closePath()
            context.fill()

            // Draw FR wheel
            context.beginPath()
            context.fillStyle = wheelFRPower > 0 ? Qt.hsva(colorBaselineHue + frRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(width, 0);
            context.lineTo(width - width / 5, 0)
            context.lineTo(width - width * 2 / 5, height / 8)
            context.lineTo(width - width / 5, height / 4)
            context.lineTo(width, height / 4)
            context.closePath()
            context.fill()

            // Draw ML wheel
            context.beginPath()
            context.fillStyle = wheelMLPower > 0 ? Qt.hsva(colorBaselineHue + mlRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(0, height * 3 / 8);
            context.lineTo(width / 5, height * 3 / 8)
            context.lineTo(width * 2 / 5, height / 2)
            context.lineTo(width / 5, height * 5 / 8)
            context.lineTo(0, height * 5 / 8)
            context.closePath()
            context.fill()

            // Draw MR wheel
            context.beginPath()
            context.fillStyle = wheelMRPower > 0 ? Qt.hsva(colorBaselineHue + mrRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(width, height * 3 / 8);
            context.lineTo(width - width / 5, height * 3 / 8)
            context.lineTo(width - width * 2 / 5, height / 2)
            context.lineTo(width - width / 5, height * 5 / 8)
            context.lineTo(width, height * 5 / 8)
            context.closePath()
            context.fill()

            // Draw BL wheel
            context.beginPath()
            context.fillStyle = wheelBLPower > 0 ? Qt.hsva(colorBaselineHue + blRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(0, height)
            context.lineTo(width / 5, height)
            context.lineTo(width * 2 / 5, height - height / 8)
            context.lineTo(width / 5, height - height / 4)
            context.lineTo(0, height - height / 4)
            context.closePath()
            context.fill()

            // Draw BR wheel
            context.beginPath()
            context.fillStyle = wheelBRPower > 0 ? Qt.hsva(colorBaselineHue + brRatio * hueRange, colorSat, colorValue, 1) : "gray"
            context.moveTo(width, height);
            context.lineTo(width - width / 5, height)
            context.lineTo(width - width * 2 / 5, height - height / 8)
            context.lineTo(width - width / 5, height - height / 4)
            context.lineTo(width, height - height / 4)
            context.closePath()
            context.fill()
        }
    }
}
