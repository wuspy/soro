import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

ApplicationWindow {
    id: commentsWindow
    width: 640
    height: 480
    title: qsTr("Research Control - Comments")

    property alias connectionState: connectionStateGroup.state
    property alias recordingState: recordingStateGroup.state

    signal logCommentEntered(string comment)
    signal recordButtonClicked()

    // Internal properties

    property color theme_yellow: "#FBC02D"
    property color theme_red: "#d32f2f"
    property color theme_green: "#388E3C"
    property color theme_blue: "#1976D2"
    property color accentColor: "#616161"

    function recordComment(text, type) {
        commentsListModel.append({"commentText": "At " + testingTimer.elapsed + " seconds:", "messageType": "time"})
        commentsListModel.append({"commentText": text, "messageType": type})
        commentsListView.positionViewAtEnd()
        if (type === "user") {
            // Send signal to backend
            logCommentEntered(text)
        }
    }

    /*
      Theme settings
      */
    Material.theme: Material.Dark
    Material.accent: accentColor
    Universal.theme: Universal.Dark
    Universal.accent: accentColor

    /*
      Timer to keep track of how long a test has been running. Also updates
      the testing time label.
      */
    Timer {
        id: testingTimer
        interval: 1000
        repeat: true

        property int elapsed;

        onTriggered: {
            elapsed++
            var elapsedHours = Math.floor(elapsed / 3600)
            var elapsedMinutes = Math.floor((elapsed - (elapsedHours * 3600)) / 60)
            var elapsedSeconds = Math.floor((elapsed - (elapsedHours * 3600)) - (elapsedMinutes * 60));

            if (elapsedMinutes.toString().length == 1) {
                elapsedMinutes = "0" + elapsedMinutes
            }
            if (elapsedSeconds.toString().length == 1) {
                elapsedSeconds = "0" + elapsedSeconds
            }

            var timeString = elapsedHours + ":" + elapsedMinutes + ":" + elapsedSeconds

            recordButtonLabel.text = timeString
        }
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
                    target: commentsWindow
                    accentColor: theme_yellow
                }
            },
            State {
                name: "connected"
                PropertyChanges {
                    target: commentsWindow
                    accentColor: theme_green
                }
            },
            State {
                name: "error"
                PropertyChanges {
                    target: commentsWindow
                    accentColor: theme_red
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
      State group to update the UI to reflect a change in the rover's connection status
      Can be accessed from the backend with the connectionState property
      */
    StateGroup {
        id: recordingStateGroup
        state: "idle"
        states: [
            State {
                name: "recording"
                PropertyChanges {
                    target: recordButtonImage
                    source: "qrc:/icons/ic_stop_white_48px.svg"
                    visible: true
                    width: 40
                }
                PropertyChanges {
                    target: recordButtonTooltip
                    text: qsTr("Stop Logging")
                    visible: true
                }
                PropertyChanges {
                    target: recordButtonLabel
                    leftPadding: 5
                    rightPadding: 5
                    text: "0:00:00"
                    visible: true
                }
                PropertyChanges {
                    target: recordButtonBusyIndicator
                    visible: false
                    width: 0
                }
            },
            State {
                name: "idle"
                PropertyChanges {
                    target: recordButtonImage
                    source: "qrc:/icons/ic_play_arrow_white_48px.svg"
                    visible: true
                    width: 40
                }
                PropertyChanges {
                    target: recordButtonTooltip
                    text: qsTr("Begin Logging")
                }
                PropertyChanges {
                    target: recordButtonLabel
                    leftPadding: 0
                    rightPadding: 0
                    text: ""
                }
                PropertyChanges {
                    target: recordButtonBusyIndicator
                    visible: false
                    width: 0
                }
            },
            State {
                name: "waiting"
                PropertyChanges {
                    target: recordButtonBusyIndicator
                    visible: true
                    width: 40
                }
                PropertyChanges {
                    target: recordButtonImage
                    visible: false
                    width: 0
                }
                PropertyChanges {
                    target: recordButtonLabel
                    leftPadding: 0
                    rightPadding: 0
                    text: ""
                }
                PropertyChanges {
                    target: recordButtonTooltip
                    text: qsTr("Waiting...")
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

    onRecordingStateChanged: {
        commentsTextArea.enabled = recordingState === "recording"
        testingTimer.restart()
        testingTimer.elapsed = 0
        testingTimer.running = recordingState === "recording"
        recordButtonMouseArea.state = recordingState === "recording" ? "checked" : "unchecked"
        if (recordingState === "recording") {
            // Clear comments since a new test is starting
            commentsListModel.clear()
        }
    }

    Item {
        id: commentsPane
        anchors.top: headerPane.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: 8
        anchors.leftMargin: 8
        anchors.rightMargin: 8

        ListView {
            id: commentsListView
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height * 0.7
            clip: true

            /* This list model accepts two parameters:
              - text: the comment text
              - type: can be either time, system, or user. Use user for user-entered comments,
               and system for system status messages.
               */
            model: ListModel {
                id: commentsListModel


            }

            delegate:Label {
                width: parent.width
                text: commentText
                topPadding: 10
                leftPadding: 10
                rightPadding: 10
                bottomPadding: timeMessage ? 5 : 10
                wrapMode: Text.WordWrap
                color: messageType == "time" ? Material.accent : Material.foreground
                font.italic: messageType == "time"
                font.bold: messageType == "system"
            }
        }

        GroupBox {
            anchors.top: commentsListView.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: 10
            TextArea {
                id: commentsTextArea
                anchors.fill: parent
                placeholderText: "Enter your comments here"
                wrapMode: Text.WordWrap
                enabled: false

                Keys.onReturnPressed: {
                    recordComment(text.trim(), "user")
                    text = "";
                }
            }
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

        Label {
            id: headerLabel
            y: 16
            text: qsTr("Comments")
            font.pointSize: 22
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 12
        }

        /* This is the test start/stop button. It also shows the elapsed time during a test,
          which it calculates itself using a timer. This doesn't need to be completely accurate
          as it only serves as a reference, the actual testing timestamps are saved elsewhere.
          */
        MouseArea {
            id: recordButtonMouseArea
            height: 40
            width: recordButtonImage.width + recordButtonLabel.width + recordButtonBusyIndicator.width
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: {
                if (recordingState !== "waiting") {
                    recordingState = "waiting"
                    recordButtonClicked()
                }
            }

            BusyIndicator {
                id: recordButtonBusyIndicator
                width: 0
                height: 40
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                visible: false
                Material.accent: Material.foreground
                Universal.accent: Universal.foreground
            }

            ToolTip {
                id: recordButtonTooltip
                visible: recordButtonMouseArea.containsMouse
                delay: 500
                timeout: 5000
            }

            Image {
                id: recordButtonImage
                sourceSize.height: height
                sourceSize.width: width
                fillMode: Image.PreserveAspectFit
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 40
                height: 40
            }

            Label {
                id: recordButtonLabel
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: recordButtonImage.right
                text: ""
                font.pointSize: 22
            }

            ColorOverlay {
                id: recordButtonColorOverlay
                anchors.fill: recordButtonImage
                source: recordButtonImage
                color: "#ffffff"
            }
        }
    }
}
