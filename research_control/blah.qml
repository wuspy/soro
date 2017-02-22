import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtWebEngine 1.3
import QtQuick.Window 2.2

GroupBox {
    id: avGroupBox
    title: qsTr("Audio/Video")
    clip: false
    width: 400
    height: 1000

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
        id: activeCameraLabel
        y: 125
        width: 100
        text: qsTr("Active Camera")
        anchors.verticalCenter: activeCameraCombo.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
    }

    ComboBox {
        id: videoEncodingCombo
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
        anchors.verticalCenter: videoEncodingCombo.verticalCenter
        anchors.leftMargin: 0
        anchors.left: parent.left
    }

    ComboBox {
        id: videoResolutionCombo
        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: videoResolutionLabel.right
        anchors.leftMargin: 12
        anchors.top: videoEncodingCombo.bottom
        anchors.topMargin: 8
    }

    Label {
        id: videoResolutionLabel
        width: 100
        text: qsTr("Resolution")
        anchors.verticalCenter: videoResolutionCombo.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
    }

    SpinBox {
        id: videoBitrateSpinBox
        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
        visible: videoEncodingCombo.currentText != "MJPEG"
        stepSize: 10
        to: 10000
        from: 500
        value: 500
        anchors.left: videoBitrateLabel.right
        anchors.leftMargin: 12
        anchors.top: videoResolutionCombo.bottom
        anchors.topMargin: 8
    }

    Label {
        id: videoBitrateLabel
        width: 100
        visible: videoBitrateSpinBox.visible
        text: qsTr("Bitrate (Kb/s)")
        anchors.verticalCenter: videoBitrateSpinBox.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
    }

    SpinBox {
        id: mjpegQualitySpinBox
        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
        visible: videoEncodingCombo.currentText == "MJPEG"
        to: 100
        from: 1
        value: 50
        anchors.left: mjpegQualityLabel.right
        anchors.leftMargin: 12
        anchors.top: videoBitrateSpinBox.bottom
        anchors.topMargin: 8
    }

    Label {
        id: mjpegQualityLabel
        visible: mjpegQualitySpinBox.visible
        width: 100
        text: qsTr("Quality (%)")
        anchors.verticalCenter: mjpegQualitySpinBox.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
    }

    SpinBox {
        id: videoFramerateSpinBox
        enabled: enableVideoSwitch.enabled & enableVideoSwitch.checked
        to: 30
        value: 0
        stepSize: 1
        anchors.left: videoFramerateLabel.right
        anchors.leftMargin: 12
        anchors.top: mjpegQualitySpinBox.bottom
        anchors.topMargin: 8
    }

    Label {
        id: videoFramerateLabel
        width: 100
        text: qsTr("Framerate (fps)")
        anchors.verticalCenter: videoFramerateSpinBox.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 0
    }

    Switch {
        id: stereoVideoSwitch
        text: checked ? qsTr("Stereo On") : qsTr("Stereo Off")
        enabled: stereoUiSwitch.checked & enableVideoSwitch.enabled & enableVideoSwitch.checked
        anchors.left: stereoVideoLabel.right
        anchors.leftMargin: 12
        anchors.top: videoFramerateSpinBox.bottom
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
}
