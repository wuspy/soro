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

import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtWebEngine 1.3
import QtQuick.Window 2.2

ApplicationWindow {
    id: mainWindow
    width: 1000
    height: 1200
    property alias driveStatusField: driveStatusField
    property alias gamepadField: gamepadField
    property alias roverAddressField: roverAddressField
    property alias notificationTitleLabel: notificationTitleLabel
    title: "Research Control"

    // Alias properties

    property alias simulationGroupBox: simulationGroupBox
    property alias avGroupBox: avGroupBox
    property alias settingsPane: settingsPane
    property alias statusImage: statusImage
    property alias notificationImageColorOverlay: notificationImageColorOverlay
    property alias notificationPane: notificationPane
    property alias notificationImage: notificationImage
    property alias notificationLabel: notificationLabel
    property alias busyIndicator: busyIndicator
    property alias statusLabel: statusLabel
    property alias bitrateLabel: bitrateLabel
    property alias revertSettingsButton: revertSettingsButton
    property alias applySettingsButton: applySettingsButton
    property alias simLatencySpinBox: simLatencySpinBox
    property alias enableHudSwitch: enableHudSwitch
    property alias stereoUiSwitch: stereoUiSwitch
    property alias hudParallaxSpinBox: hudParallaxSpinBox
    property alias activeCameraCombo: activeCameraCombo
    property alias videoFormatCombo: videoFormatCombo
    property alias stereoVideoSwitch: stereoVideoSwitch
    property alias enableAudioSwitch: enableAudioSwitch
    property alias enableVideoSwitch: enableVideoSwitch
    property alias gpsGroupBox: gpsGroupBox
    property alias gpsClearButton: gpsClearButton
    property alias gpsHistoryField: gpsHistoryField
    property alias gpsLocationField: gpsLocationField
    property alias enableGpsSwitch: enableGpsSwitch
    property alias webEngineView: webEngineView
    property alias settingsFooterPane: settingsFooterPane
    property alias connectionState: connectionStateGroup.state
    property alias recordingState: recordingStateGroup.state
    property alias fullscreenState: fullscreenStateGroup.state
    property alias sidePaneVisibilityState: sidePaneVisibilityStateGroup.state
    property alias fullscreenToolbarButton: fullscreenToolbarButton
    property alias recordToolbarButton: recordToolbarButton
    property alias sidebarToolbarButton: sidebarToolbarButton
    property alias hudLatencySpinBox: hudLatencySpinBox

    // Settings properties

    property alias enableStereoUi: stereoUiSwitch.checked
    property alias enableVideo: enableVideoSwitch.checked
    property alias enableStereoVideo: stereoVideoSwitch.checked
    property alias enableHud: enableHudSwitch.checked
    property alias selectedCamera: activeCameraCombo.currentIndex
    property alias selectedVideoFormat: videoFormatCombo.currentIndex
    property alias enableAudio: enableAudioSwitch.checked
    property alias selectedLatency: simLatencySpinBox.value
    property alias selectedHudParallax: hudParallaxSpinBox.value
    property alias enableGps: enableGpsSwitch.checked
    property alias selectedHudLatency: hudLatencySpinBox.value

    // Configuration properties

    property alias videoFormatNames: videoFormatCombo.model
    property alias cameraNames: activeCameraCombo.model
    property string roverAddress: "0.0.0.0"
    property string gamepad: "None"
    property string driveStatus: "Unknown"

    // Internal properties

    property color theme_yellow: "#FBC02D"
    property color theme_red: "#d32f2f"
    property color theme_green: "#388E3C"
    property color theme_blue: "#1976D2"
    property color accentColor: "#616161"
    property int gpsDataPoints: 0

    // Signals

    signal requestUiSync()
    signal settingsApplied()
    signal logCommentEntered(string comment)
    signal recordButtonClicked()
    signal closed()

    // Public functions

    /*
      Should be called before the backend has started syncing
      the state of the UI to reflect the current settings
      */
    function prepareForUiSync() {
        settingsPane.enabled = false
        settingsFooterPane.visible = false
    }

    /*
      Should be called when the backend has finished syncing the
      state of the UI to reflect the current settings
      */
    function uiSyncComplete() {
        settingsFooterPane.state = "hidden"
        settingsPane.enabled = true
        settingsFooterPane.visible = true
    }

    /*
      Updates the ping (latency) information displayed
      */
    function updatePing(ping) {
        if (connectionState == "connected") {
            statusLabel.text = "Connected, " + ping + "ms"
        }
    }

    /*
      Updates the GPS location displayed on the map
      */
    function updateGpsLocation(lat, lng, heading) {
        webEngineView.runJavaScript("updateLocation(" + lat + ", " + lng + ", " + heading + ");")
        gpsLocationField.text = degToDms(lat, false) + ", " + degToDms(lng, true)
        gpsDataPoints++
    }

    /*
      Updates the bitrate status label
      */
    function updateBitrate(bpsUp, bpsDown) {
        if (connectionState == "connected") {
            var upUnits, downUnits;
            if (bpsUp > 1000000) {
                upUnits = "Mb/s"
                bpsUp = Math.round(bpsUp / 10000) / 100
            }
            else if (bpsUp > 1000) {
                upUnits = "Kb/s"
                bpsUp = Math.round(bpsUp / 10) / 100
            }
            else {
                upUnits = "b/s"
            }
            if (bpsDown > 1000000) {
                downUnits = "Mb/s"
                bpsDown = Math.round(bpsDown / 10000) / 100
            }
            else if (bpsDown > 1000) {
                downUnits = "Kb/s"
                bpsDown = Math.round(bpsDown / 10) / 100
            }
            else {
                downUnits = "b/s"
            }
            bitrateLabel.text =
                    "▲ <b>" + bpsUp + "</b> " + upUnits +
                    "<br>" +
                    "▼ <b>" + bpsDown + "</b> " + downUnits
        }
    }

    /*
      Displays a popup notification for a few seconds.
      Valid types are: 'error', 'warning', or 'information'
      */
    function notify(type, title, message) {
        notificationPane.state = "hidden"
        switch (type) {
        case "error":
            notificationImage.source = "qrc:/icons/ic_error_white_48px.svg"
            notificationImageColorOverlay.color = theme_red
            break
        case "warning":
            notificationImage.source = "qrc:/icons/ic_warning_white_48px.svg"
            notificationImageColorOverlay.color = theme_yellow
            break;
        case "information":
        default:
            notificationImage.source = "qrc:/icons/ic_info_white_48px.svg"
            notificationImageColorOverlay.color = theme_blue
            break;
        }
        notificationTitleLabel.text = title
        notificationLabel.text = message
        notificationPane.state = "visible"
        notificationTimer.restart()
    }

    /*
      Internal function to convert degress to degree/minute/seconds
      */
    function degToDms(D, lng){
        return "" + 0|(D<0?D=-D:D) + "° "
                + 0|D%1*60 + "' "
                + (0|D*60%1*6000)/100 + "\" "
                + D<0?lng?'W':'S':lng?'E':'N'
    }

    /*
      Reloads the GPS map to clear all location points currently displayed
      */
    function clearGps() {
        gpsDataPoints = 0
        webEngineView.runJavaScript("resetLocations()");
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    /*
      Theme settings
      */
    Material.theme: Material.Dark
    Material.accent: accentColor
    Universal.theme: Universal.Dark
    Universal.accent: accentColor

    onVisibleChanged: {
        if (visible) {
            // Request for the UI to be synced since it just became visible
            prepareForUiSync()
            requestUiSync()
        }
    }

    onGpsDataPointsChanged: {
        // Update the gps history field to reflect the current # of samples
        gpsHistoryField.text = "" + gpsDataPoints + " Samples"
    }

    onRoverAddressChanged: {
        // Update the rover address field
        roverAddressField.text = roverAddress
    }

    onGamepadChanged: {
        // Update the gamepad field
        if (gamepad == "") {
            gamepadField.text = "None"
        }
        else {
            gamepadField.text = gamepad
        }
    }

    onDriveStatusChanged: {
        // Update the drive status field
        driveStatusField.text = driveStatus
    }

    onClosing: {
        if (recordingState !== "idle") {
            close.accepted = false
            onWindowCloseDialog.visible = true
        }
        else {
            closed()
        }
    }

    StateGroup {
        id: fullscreenStateGroup
        state: "normal"
        states: [
            State {
                name: "fullscreen"
                PropertyChanges {
                    target: fullscreenToolbarButton
                    tooltip.text: "Exit Fullscreen"
                    image.source: "qrc:/icons/ic_fullscreen_exit_white_48px.svg"
                }
                StateChangeScript {
                    script: mainWindow.showFullScreen()
                }
            },
            State {
                name: "normal"
                PropertyChanges {
                    target: fullscreenToolbarButton
                    tooltip.text: "Show Fullscreen"
                    image.source: "qrc:/icons/ic_fullscreen_white_48px.svg"
                }
                StateChangeScript {
                    script: mainWindow.showNormal()
                }
            }
        ]
    }


    /* State group to control the visibility of the options sidebar
      */
    StateGroup {
        id: sidePaneVisibilityStateGroup
        state: "visible"

        states: [
            State {
                name: "hidden"
                PropertyChanges {
                    target: asidePane
                    anchors.leftMargin: -asidePane.width
                }
                PropertyChanges {
                    target: sidebarToolbarButton
                    tooltip.text: "Show Sidebar"
                    image.source: "qrc:/icons/ic_keyboard_arrow_right_white_48px.svg"
                }
            },
            State {
                name: "visible"
                PropertyChanges {
                    target: asidePane
                    anchors.leftMargin: 0
                }
                PropertyChanges {
                    target: sidebarToolbarButton
                    tooltip.text: "Hide Sidebar"
                    image.source: "qrc:/icons/ic_keyboard_arrow_left_white_48px.svg"
                }
            }

        ]

        transitions: [
            Transition {
                from: "visible"
                to: "hidden"
                PropertyAnimation {
                    properties: "anchors.leftMargin"
                    duration: 100
                }
            },
            Transition {
                from: "hidden"
                to: "visible"
                PropertyAnimation {
                    properties: "anchors.leftMargin"
                    duration: 100
                }
            }
        ]
    }

    /*
      State group to update the UI to reflect a change in the rover's connection status
      Can be accessed from the backend with the connectionState property
      */
    StateGroup {
        id: connectionStateGroup
        state: "connecting"
        states: [
            State {
                name: "connecting"
                PropertyChanges {
                    target: mainWindow
                    accentColor: theme_yellow
                    busyIndicator.visible: true
                    statusImage.visible: false
                    bitrateLabel.visible: false
                    statusLabel.text: "Connecting..."
                    avGroupBox.enabled: false
                    simulationGroupBox.enabled: false
                    gpsGroupBox.enabled: false
                }
            },
            State {
                name: "connected"
                PropertyChanges {
                    target: mainWindow
                    accentColor: theme_green
                    busyIndicator.visible: false
                    statusImage.visible: true
                    bitrateLabel.visible: true
                    statusLabel.text: "Connected"
                    statusImage.source: "qrc:/icons/ic_check_circle_white_48px.svg"
                    avGroupBox.enabled: true
                    simulationGroupBox.enabled: true
                    gpsGroupBox.enabled: true
                }
            },
            State {
                name: "error"
                PropertyChanges {
                    target: mainWindow
                    accentColor: theme_red
                    busyIndicator.visible: false
                    statusImage.visible: true
                    bitrateLabel.visible: false
                    statusLabel.text: "Error"
                    statusImage.source: "qrc:/icons/ic_error_white_48px.svg"
                    settingsPane.enabled: false
                }
            }
        ]
        transitions: [
            Transition {
                ColorAnimation {
                    duration: 250
                }
            }
        ]
    }

    /*
      State group to show the recording status in the UI.
      */
    StateGroup {
        id: recordingStateGroup
        state: "idle"
        states: [
            State {
                name: "recording"
                PropertyChanges {
                    target: recordToolbarButton
                    state: "recording"
                }
                PropertyChanges {
                    target: recordingTimer
                    running: true
                    elapsed: 0
                }
                StateChangeScript {
                    script: clearGps()
                }
            },
            State {
                name: "idle"
                PropertyChanges {
                    target: recordToolbarButton
                    state: "idle"
                }
                PropertyChanges {
                    target: recordingTimer
                    running: false
                }
            },
            State {
                name: "waiting"
                PropertyChanges {
                    target: recordToolbarButton
                    state: "waiting"
                }
                PropertyChanges {
                    target: recordingTimer
                    running: false
                }
            }
        ]
    }

    onConnectionStateChanged: {
        if (recordingState === "recording") {
            // Append the status to the test comment log
            switch (connectionState) {
            case "connected":
                recordComment("[System Message] The rover is now connected", "system");
                break;
            case "error":
                recordComment("[System Message] The communication channel with the rover has experienced a fatal error", "system");
                break;
            case "connecting":
            default:
                recordComment("[System Message] Connection to the rover has been lost", "system");
                break;
            }
        }
    }

    /*
      Timer to dismiss notifications after a set amout of time
      */
    Timer {
        id: notificationTimer
        interval: 7000
        running: false
        repeat: false
        onTriggered: notificationPane.state = "hidden"
    }

    RecordingTimer {
        id: recordingTimer

        onTimeStringChanged: {
            recordToolbarButton.label.text = timeString
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    WebEngineView {
        id: webEngineView
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: asidePane.right
        anchors.bottom: parent.bottom
        url: "qrc:/html/map.html"
    }

    DropShadow {
        id: asideShadow
        anchors.fill: asidePane
        source: asidePane
        radius: 15
        samples: 20
        color: "#000000"
    }

    Pane {
        id: asidePane
        width: 450
        bottomPadding: 0
        rightPadding: 0
        leftPadding: 0
        topPadding: 0
        clip: true
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: headerPane.bottom
        anchors.topMargin: 0

        Flickable {
            id: settingsFlickable
            clip: false
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.bottomMargin: 8
            anchors.bottom: settingsFooterPane.state == "visible" ? settingsFooterPane.top : parent.bottom
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 8
            contentHeight: contentItem.childrenRect.height

            Pane {
                id: settingsPane
                width: parent.width
                height: gpsGroupBox.y + gpsGroupBox.height + topPadding + bottomPadding
                x: 0
                y: 0

                GroupBox {
                    id: infoGroupBox
                    width: 200
                    height: driveStatusField.y + driveStatusField.height + topPadding + bottomPadding
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.top: parent.top
                    anchors.topMargin: 0
                    title: qsTr("Information")

                    Label {
                        id: roverAddressLabel
                        y: 6
                        width: 100
                        text: qsTr("Rover Address")
                        anchors.verticalCenter: roverAddressField.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Label {
                        id: roverAddressField
                        text: qsTr("Unknown")
                        anchors.left: roverAddressLabel.right
                        anchors.leftMargin: 12
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.top: parent.top
                        anchors.topMargin: 0
                    }

                    Label {
                        id: gamepadLabel
                        y: 55
                        width: 100
                        text: qsTr("Input Device")
                        anchors.verticalCenter: gamepadField.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Label {
                        id: gamepadField
                        text: qsTr("None")
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: gamepadLabel.right
                        anchors.leftMargin: 12
                        anchors.top: roverAddressField.bottom
                        anchors.topMargin: 8
                    }

                    Label {
                        id: driveStatusLabel
                        x: 5
                        y: 60
                        width: 100
                        text: qsTr("Drive Status")
                        anchors.verticalCenterOffset: 0
                        anchors.leftMargin: 0
                        anchors.verticalCenter: driveStatusField.verticalCenter
                        anchors.left: parent.left
                    }

                    Label {
                        id: driveStatusField
                        text: qsTr("Unknown")
                        anchors.leftMargin: 12
                        anchors.left: driveStatusLabel.right
                        anchors.rightMargin: 0
                        anchors.top: gamepadField.bottom
                        anchors.topMargin: 8
                        anchors.right: parent.right
                    }
                }

                GroupBox {
                    id: interfaceGroupBox
                    x: -12
                    y: -12
                    height: interfaceNotesLabel.y + interfaceNotesLabel.height + topPadding + bottomPadding
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.top: infoGroupBox.bottom
                    anchors.topMargin: 8
                    title: qsTr("User Interface")

                    Label {
                        id: stereoUiLabel
                        width: 100
                        height: 18
                        text: qsTr("Stereo UI")
                        anchors.verticalCenter: stereoUiSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Switch {
                        id: stereoUiSwitch
                        text: checked ? qsTr("Stereo On") : qsTr("Stereo Off")
                        anchors.left: stereoUiLabel.right
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        onCheckedChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: enableHudLabel
                        y: 17
                        width: 100
                        text: qsTr("Enable HUD")
                        anchors.verticalCenter: enableHudSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Switch {
                        id: enableHudSwitch
                        text: checked ? qsTr("HUD On") : qsTr("HUD Off")
                        anchors.left: enableHudLabel.right
                        anchors.leftMargin: 12
                        anchors.top: stereoUiSwitch.bottom
                        anchors.topMargin: 8
                        onCheckedChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: hudParallaxLabel
                        y: 27
                        width: 100
                        text: qsTr("HUD Parallax")
                        anchors.verticalCenter: hudParallaxSpinBox.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    SpinBox {
                        id: hudParallaxSpinBox
                        anchors.left: hudParallaxLabel.right
                        anchors.leftMargin: 12
                        anchors.top: enableHudSwitch.bottom
                        anchors.topMargin: 0
                        stepSize: 10
                        editable: true
                        from: 0
                        to: 500
                        onValueChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: hudLatencyLabel
                        y: 27
                        width: 100
                        text: qsTr("HUD Latency (ms)")
                        anchors.verticalCenter: hudLatencySpinBox.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    SpinBox {
                        id: hudLatencySpinBox
                        anchors.left: hudLatencyLabel.right
                        anchors.leftMargin: 12
                        anchors.top: hudParallaxSpinBox.bottom
                        anchors.topMargin: 0
                        stepSize: 10
                        editable: true
                        from: 0
                        to: 2000
                        onValueChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: interfaceNotesLabel
                        text: qsTr("<b>Stereo UI</b> renders the interface in side-by-side stereo.  Video will not be streamed in stereo unless the 'Stereo Video' option is also selected.<br><br><b>HUD Parallax</b> changes the how far away the HUD appears to the viewer in stereo mode. Lower values make it appear farther away, and higher values make it appear closer.<br><br><b>HUD Latency</b> adds extra latency to the HUD latency visualizer if needed to better sync it with the actual rover response.")
                        textFormat: Text.RichText
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.top: hudLatencySpinBox.bottom
                        anchors.topMargin: 8
                        wrapMode: Text.WordWrap
                        verticalAlignment: Text.AlignBottom
                    }
                }

                GroupBox {
                    id: avGroupBox
                    x: -12
                    y: 200
                    height: videoNotesLabel.y + videoNotesLabel.height + topPadding + bottomPadding
                    title: qsTr("Audio/Video")
                    clip: false
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.top: interfaceGroupBox.bottom
                    anchors.topMargin: 8

                    Switch {
                        id: stereoVideoSwitch
                        text: checked ? qsTr("Stereo On") : qsTr("Stereo Off")
                        enabled: stereoUiSwitch.checked & enableVideoSwitch.enabled & enableVideoSwitch.checked
                        anchors.left: stereoVideoLabel.right
                        anchors.leftMargin: 12
                        anchors.top: videoFormatCombo.bottom
                        anchors.topMargin: 8
                        onCheckedChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: stereoVideoLabel
                        width: 100
                        height: 18
                        text: qsTr("Stereo Video")
                        anchors.verticalCenter: stereoVideoSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    ComboBox {
                        id: videoFormatCombo
                        textRole: qsTr("")
                        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
                        anchors.left: videoEncodingLabel.right
                        anchors.leftMargin: 12
                        anchors.top: activeCameraCombo.bottom
                        anchors.topMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        onCurrentIndexChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: videoEncodingLabel
                        width: 100
                        text: qsTr("Encoding")
                        anchors.verticalCenter: videoFormatCombo.verticalCenter
                        anchors.leftMargin: 0
                        anchors.left: parent.left
                    }


                    Label {
                        id: activeCameraLabel
                        y: 125
                        width: 100
                        text: qsTr("Active Camera")
                        anchors.verticalCenter: activeCameraCombo.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }


                    ComboBox {
                        id: activeCameraCombo
                        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.top: enableAudioSwitch.bottom
                        anchors.topMargin: 8
                        anchors.left: activeCameraLabel.right
                        anchors.leftMargin: 12
                        onCurrentIndexChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: videoNotesLabel
                        text: qsTr("<b>Stereo Video</b> has the same target bitrate as mono video, with half the horizontal resolution per eye.")
                        textFormat: Text.RichText
                        verticalAlignment: Text.AlignBottom
                        anchors.top: stereoVideoSwitch.bottom
                        anchors.topMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        wrapMode: Text.WordWrap
                    }

                    Switch {
                        id: enableVideoSwitch
                        y: 96
                        text: checked ? qsTr("Video On") : qsTr("Video Off")
                        anchors.left: enableVideoLabel.right
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        onCheckedChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: enableVideoLabel
                        y: 77
                        width: 100
                        text: qsTr("Enable Video")
                        anchors.verticalCenter: enableVideoSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Switch {
                        id: enableAudioSwitch
                        x: 100
                        y: 248
                        text: checked ? qsTr("Audio On") : qsTr("Audio Off")
                        anchors.left: enableAudioLabel.right
                        anchors.leftMargin: 12
                        anchors.top: enableVideoSwitch.bottom
                        anchors.topMargin: 8
                        onCheckedChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: enableAudioLabel
                        x: -12
                        y: 259
                        width: 100
                        text: qsTr("Enable Audio")
                        anchors.verticalCenter: enableAudioSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }
                }

                GroupBox {
                    id: simulationGroupBox
                    height: simNotesLabel.y + simNotesLabel.height + topPadding + bottomPadding
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.top: avGroupBox.bottom
                    anchors.topMargin: 8
                    title: qsTr("Simulation")

                    Label {
                        id: simLatencyLabel
                        y: 27
                        width: 100
                        text: qsTr("Latency (ms)")
                        anchors.verticalCenter: simLatencySpinBox.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    SpinBox {
                        id: simLatencySpinBox
                        anchors.left: simLatencyLabel.right
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        stepSize: 50
                        editable: true
                        from: 0
                        to: 10000
                        onValueChanged: settingsFooterPane.state = "visible"
                    }

                    Label {
                        id: simNotesLabel
                        text: qsTr("<b>Latency</b> only delays driving commands. Any operations performed on this screen will be unaffected.")
                        anchors.top: simLatencySpinBox.bottom
                        anchors.topMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        textFormat: Text.RichText
                        wrapMode: Text.WordWrap
                    }
                }

                GroupBox {
                    id: gpsGroupBox
                    width: 200
                    height: gpsHistoryField.y + gpsHistoryField.height + topPadding + bottomPadding
                    anchors.top: simulationGroupBox.bottom
                    anchors.topMargin: 8
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    title: qsTr("GPS")

                    Label {
                        id: enableGpsLabel
                        y: 1
                        width: 100
                        text: qsTr("Enable GPS")
                        anchors.verticalCenter: enableGpsSwitch.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                    }

                    Switch {
                        id: enableGpsSwitch
                        text: checked ? qsTr("GPS On") : qsTr("GPS Off")
                        anchors.left: enableGpsLabel.right
                        anchors.leftMargin: 12
                        anchors.top: parent.top
                        anchors.topMargin: 0
                    }

                    Label {
                        id: gpsLocationLabel
                        y: 76
                        width: 100
                        text: qsTr("Last Location")
                        anchors.left: parent.left
                        anchors.leftMargin: 0
                        anchors.verticalCenter: gpsLocationField.verticalCenter
                    }

                    TextField {
                        id: gpsLocationField
                        readOnly: true
                        text: qsTr("Nothing Received")
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: gpsLocationLabel.right
                        anchors.leftMargin: 12
                        anchors.top: enableGpsSwitch.bottom
                        anchors.topMargin: 8
                    }

                    Label {
                        id: gpsHistoryLabel
                        x: 0
                        y: 76
                        width: 100
                        text: qsTr("Data Points")
                        anchors.verticalCenterOffset: 0
                        anchors.leftMargin: 0
                        anchors.verticalCenter: gpsHistoryField.verticalCenter
                        anchors.left: parent.left
                    }

                    TextField {
                        id: gpsHistoryField
                        text: qsTr("Nothing Received")
                        anchors.leftMargin: 12
                        anchors.left: gpsHistoryLabel.right
                        anchors.rightMargin: 12
                        anchors.top: gpsLocationField.bottom
                        anchors.topMargin: 8
                        anchors.right: gpsClearButton.left
                        readOnly: true
                    }

                    Button {
                        id: gpsClearButton
                        x: 296
                        y: 142
                        text: qsTr("Clear")
                        anchors.verticalCenter: gpsHistoryField.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        onClicked: clearGps()
                    }
                }

            }

            ScrollIndicator.vertical:  ScrollIndicator { }
        }

        DropShadow {
            id: footerShadow
            anchors.fill: settingsFooterPane
            visible: settingsFooterPane.visible
            source: settingsFooterPane
            horizontalOffset: -15
            radius: 15
            samples: 20
            color: "#000000"
        }

        Pane {
            id: settingsFooterPane
            //Material.theme: Material.Light
            //Universal.theme: Universal.Light
            height: 64
            visible: true
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.bottom: parent.bottom
            state: "hidden"

            Button {
                id: applySettingsButton
                text: qsTr("Apply")
                Material.background: Material.accent
                Universal.background: Universal.accent
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 0
                onClicked: {
                    settingsFooterPane.state = "hidden"
                    settingsApplied()
                }
            }

            Label {
                id: settingsFooterLabel
                text: qsTr("Settings have been changed.")
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: revertSettingsButton.left
                anchors.rightMargin: 12
                anchors.left: parent.left
                anchors.leftMargin: 0
            }

            Button {
                id: revertSettingsButton
                text: qsTr("Revert")
                anchors.rightMargin: 12
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.right: applySettingsButton.left
                onClicked: {
                    settingsFooterPane.state = "hidden"
                    requestUiSync()
                }
            }

            states: [
                State{
                    name: "hidden"
                    PropertyChanges {
                        target: settingsFooterPane
                        anchors.bottomMargin: -height - footerShadow.radius
                    }
                },
                State {
                    name: "visible"
                    PropertyChanges {
                        target: settingsFooterPane
                        anchors.bottomMargin: 0
                    }
                }
            ]

            transitions: [
                Transition {
                    PropertyAnimation {
                        properties: "anchors.bottomMargin"
                        duration: 250
                        easing.type: Easing.InOutExpo
                    }
                }
            ]
        }

    }

    DropShadow {
        id: headerShadow
        anchors.fill: headerPane
        visible: headerPane.visible
        source: headerPane
        radius: 15
        samples: 20
        color: "#000000"
    }

    Pane {
        id: headerPane
        Material.background: Material.accent
        Material.foreground: "#ffffff"
        Universal.background: Universal.accent
        Universal.foreground: "#ffffff"
        height: 64
        clip: false
        bottomPadding: 12
        rightPadding: 12
        leftPadding: 12
        topPadding: 12
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0

        BusyIndicator {
            id: busyIndicator
            width: 40
            height: 40
            visible: true
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.verticalCenter: parent.verticalCenter
            Material.accent: Material.foreground
            Universal.accent: Universal.foreground
        }

        Image {
            id: statusImage
            width: 40
            height: 40
            visible: false
            sourceSize.height: height
            sourceSize.width: width
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.verticalCenter: parent.verticalCenter
        }

        ColorOverlay {
            id: statusImageColorOverlay
            anchors.fill: statusImage
            source: statusImage
            color: "#ffffff"
            visible: statusImage.visible
        }

        Label {
            id: statusLabel
            y: 16
            text: qsTr("Please Wait...")
            font.pointSize: 22
            anchors.verticalCenter: busyIndicator.verticalCenter
            anchors.left: busyIndicator.right
            anchors.leftMargin: 12
        }

        Label {
            id: bitrateLabel
            x: 308
            y: 11
            text: qsTr("▲ <b>Up</b> Mb/s<br>▼ <b>Down</b> Mb/s")
            visible: false
            anchors.verticalCenterOffset: 0
            textFormat: Text.RichText
            font.pointSize: 10
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: headerSeparator.left
            anchors.rightMargin: 12
        }

        Pane {
            id: headerSeparator
            x: 437
            Material.background: Material.foreground
            Universal.background: Universal.foreground
            width: 1
            anchors.left: parent.left
            anchors.leftMargin: 438
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0
        }

        ToolbarButton {
            id: sidebarToolbarButton
            anchors.left: headerSeparator.right

            onClicked: {
                if (sidePaneVisibilityState === "visible") {
                    sidePaneVisibilityState = "hidden"
                }
                else {
                    sidePaneVisibilityState = "visible"
                }
            }
        }

        RecordButton {
            id: recordToolbarButton
            anchors.right: fullscreenToolbarButton.left
            onClicked: {
                switch (recordingState) {
                case "recording":
                    confirmRecordStopDialog.visible = true
                    break
                case "idle":
                    recordButtonClicked()
                    break
                case "waiting":
                default:
                    break
                }
            }
        }

        ToolbarButton {
            id: fullscreenToolbarButton
            anchors.right: parent.right
            onClicked: {
                if (fullscreenState === "fullscreen") {
                    fullscreenState = "normal"
                }
                else {
                    fullscreenState = "fullscreen"
                }
            }
        }
    }

    DropShadow {
        id: notificationShadow
        anchors.fill: notificationPane
        source: notificationPane
        radius: 15
        visible: notificationPane.visible
        opacity: notificationPane.opacity
        samples: 20
        color: "#000000"
    }

    Pane {
        id: notificationPane
        //Material.theme: Material.Light
        //Universal.theme: Universal.Light
        x: 800
        y: 181
        width: 400
        height: 100
        state: "hidden"
        anchors.right: parent.right


        Label {
            id: notificationTitleLabel
            text: qsTr("Network Driving Disabled")
            font.pointSize: 12
            font.bold: true
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: notificationImage.right
            anchors.leftMargin: 12
            anchors.top: parent.top
            anchors.topMargin: 0
        }

        Label {
            id: notificationLabel
            text: qsTr("The rover is being driven by serial override. Network drive commands will not be accepted.")
            anchors.topMargin: 0
            anchors.leftMargin: 12
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.right: parent.right
            anchors.left: notificationImage.right
            anchors.top: notificationTitleLabel.bottom
            verticalAlignment: Text.AlignTop
            wrapMode: Text.WordWrap
            font.pointSize: 10
        }


        Image {
            id: notificationImage
            width: height
            sourceSize.height: height
            sourceSize.width: width
            fillMode: Image.PreserveAspectFit
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0
            source: "qrc:/icons/ic_info_white_48px.svg"
        }


        ColorOverlay {
            id: notificationImageColorOverlay
            anchors.fill: notificationImage
            source: notificationImage
            color: "#ff0000"
        }


        MouseArea {
            id: notificationMouseArea
            anchors.fill: parent
            onClicked: notificationPane.state = "hidden"
            cursorShape: Qt.PointingHandCursor
            anchors.topMargin: -notificationPane.topPadding
            anchors.bottomMargin: -notificationPane.bottomPadding
            anchors.leftMargin: -notificationPane.leftPadding
            anchors.rightMargin: -notificationPane.rightPadding
        }


        states: [
            State {
                name: "hidden"
                PropertyChanges {
                    target: notificationPane
                    anchors.rightMargin: -width - notificationShadow.radius
                    opacity: 0
                }
            },
            State {
                name: "visible"
                PropertyChanges {
                    target: notificationPane
                    anchors.rightMargin: 0
                    opacity: 0.7
                }
            }

        ]

        transitions: [
            Transition {
                from: "hidden"
                to: "visible"
                PropertyAnimation {
                    properties: "anchors.rightMargin,opacity"
                    duration: 500
                    easing.type: Easing.InOutExpo
                }
            },
            Transition {
                from: "visible"
                to: "hidden"
                PropertyAnimation {
                    properties: "opacity"
                    duration: 100
                }
            }
        ]
    }

    ConfirmRecordStopDialog {
        id: confirmRecordStopDialog
        onAccepted: recordButtonClicked()
    }

    OnWindowCloseDialog {
        id: onWindowCloseDialog
    }
}
