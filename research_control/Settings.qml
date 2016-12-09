import QtQuick 2.4
import QtQuick.Controls 2.0

ApplicationWindow {
    id: settingsWindow

    property alias settingsForm: settingsForm

    SettingsForm {
        id: settingsForm
        anchors.fill: parent
    }
}
