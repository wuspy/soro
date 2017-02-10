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
import QtGraphicalEffects 1.0

Item {

    property int backgroundStyleTopLeft: 0
    property int backgroundStyleTop: 1
    property int backgroundStyleTopRight: 2
    property int backgroundStyleRight: 3
    property int backgroundStyleBottomRight: 4
    property int backgroundStyleBottom: 5
    property int backgroundStyleBottomLeft: 6
    property int backgroundStyleLeft: 7
    property int backgroundStyleCenter: 8

    property int backgroundStyle: backgroundStyleCenter

    property int margin: 16


    DropShadow {
        anchors.fill: backdrop
        source: backdrop
        radius: margin
        samples: radius * 2
        color: "black"
    }

    Rectangle {
        id: backdrop
        anchors.fill: parent
        anchors.margins: parent.margin
        color: "#88000000"
        border.color: "#0091EA"
        border.width: 2
        radius: 10
    }
}
