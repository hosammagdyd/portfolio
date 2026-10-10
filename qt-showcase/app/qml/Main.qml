import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtShowcase

ApplicationWindow {
    id: window
    width: 960
    height: 600
    minimumWidth: 320
    minimumHeight: 480
    visible: true
    title: qsTr("Qt Showcase — Ventilator Monitor")
    color: Theme.background

    // Below this width we switch from a side-by-side master/detail layout
    // to a single scrollable column with the detail panel stacked under the
    // device list — suited to phone screens.
    readonly property bool isNarrow: width < 700

    property Device selectedDevice: null

    DeviceListModel {
        id: deviceListModel
    }

    DeviceSimulator {
        model: deviceListModel
        running: true
    }

    Loader {
        anchors.fill: parent
        sourceComponent: window.isNarrow ? narrowLayout : wideLayout
    }

    Component {
        id: wideLayout

        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMd
            spacing: Theme.spacingMd

            DashboardView {
                Layout.preferredWidth: 300
                Layout.minimumWidth: 260
                Layout.fillHeight: true
                model: deviceListModel
                onDeviceSelected: (device) => window.selectedDevice = device
            }

            DeviceDetailView {
                Layout.fillWidth: true
                Layout.minimumWidth: 320
                Layout.fillHeight: true
                device: window.selectedDevice
            }
        }
    }

    Component {
        id: narrowLayout

        Flickable {
            contentWidth: width
            contentHeight: pageContent.implicitHeight + Theme.spacingMd * 2
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            ColumnLayout {
                id: pageContent
                x: Theme.spacingMd
                y: Theme.spacingMd
                width: parent.width - Theme.spacingMd * 2
                spacing: Theme.spacingMd

                DashboardView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight
                    scrollableInternally: false
                    model: deviceListModel
                    onDeviceSelected: (device) => window.selectedDevice = device
                }

                DeviceDetailView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight + Theme.spacingXl * 2
                    visible: window.selectedDevice !== null
                    device: window.selectedDevice
                }
            }
        }
    }
}
