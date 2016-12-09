import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Pane {
    id: root
    width: 400
    height: 600
    property alias interfaceApplyButton: interfaceApplyButton
    property alias interfaceRevertButton: interfaceRevertButton
    property alias stereoUiSwitch: stereoUiSwitch
    property alias videoRevertButton: videoRevertButton
    property alias videoApplyButton: videoApplyButton
    property alias activeCameraCombo: activeCameraCombo
    property alias videoFormatCombo: videoFormatCombo
    property alias stereoVideoSwitch: stereoVideoSwitch

    GroupBox {
        id: groupBox1
        height: 279
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: groupBox2.bottom
        anchors.topMargin: 8
        title: qsTr("Video")

        Switch {
            id: stereoVideoSwitch
            objectName: "stereoVideoSwitch"
            text: qsTr("Stereo Off")
            anchors.left: stereoVideoLabel.right
            anchors.leftMargin: 12
            anchors.top: videoFormatCombo.bottom
            anchors.topMargin: 8
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
            objectName: "videoFormatCombo"
            height: 40
            textRole: qsTr("")
            anchors.left: videoEncodingLabel.right
            anchors.leftMargin: 12
            anchors.top: activeCameraCombo.bottom
            anchors.topMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 0

            model: ["One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten"]
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
            id: videoNotesLabel
            text: qsTr("Stereo video has the same target bitrate as mono video, with half the horizontal resolution per eye.")
            anchors.bottom: videoApplyButton.top
            anchors.bottomMargin: 8
            verticalAlignment: Text.AlignBottom
            anchors.top: videoEncodingCombo.bottom
            anchors.topMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
            wrapMode: Text.WordWrap
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
            height: 40
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0
            anchors.left: activeCameraLabel.right
            anchors.leftMargin: 12
        }

        Button {
            id: videoApplyButton
            x: 252
            y: 162
            text: qsTr("Apply")
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
        }

        Button {
            id: videoRevertButton
            x: 146
            y: 211
            text: qsTr("Revert")
            anchors.right: videoApplyButton.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.rightMargin: 8
        }
    }

    GroupBox {
        id: groupBox2
        height: 201
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 0
        title: qsTr("Interface")

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
            objectName: "stereoUiSwitch"
            text: qsTr("Stereo Off")
            anchors.left: stereoUiLabel.right
            anchors.leftMargin: 12
            anchors.top: parent.top
            anchors.topMargin: 0
        }

        Label {
            id: interfaceNotesLabel
            text: qsTr("Stereo UI renders the interface in side-by-side stereo.  Video will not be streamed in stereo unless the 'Stereo Video' option is also selected.")
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.bottom: interfaceApplyButton.top
            anchors.bottomMargin: 8
            anchors.top: stereoUiSwitch.bottom
            anchors.topMargin: 8
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignBottom
        }

        Button {
            id: interfaceRevertButton
            x: 135
            y: 434
            text: qsTr("Revert")
            anchors.right: interfaceApplyButton.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.rightMargin: 8
        }

        Button {
            id: interfaceApplyButton
            x: 240
            y: 434
            text: qsTr("Apply")
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.rightMargin: 0
        }

    }
}
