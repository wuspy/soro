import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Controls.Universal 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtCharts 2.0

Item {
    ChartView {
        theme: ChartView.ChartThemeLight
        antialiasing: true
        anchors.fill: parent
        title: "People who are awesome"

        PieSeries {
            id: pieSeries
            PieSlice { label: "Jacob"; value: 100 }
            PieSlice { label: "Other People"; value: 0 }
        }
    }
}
