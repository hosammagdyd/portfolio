import QtQuick
import QtQuick.Layouts
import QtShowcase

Item {
    id: root
    property Device device: null
    clip: true

    readonly property color statusTint: {
        if (!root.device) return Theme.accent
        switch (root.device.statusText) {
        case "critical": return Theme.statusCritical
        case "warning": return Theme.statusWarning
        default: return Theme.statusNormal
        }
    }

    // Exposed so a parent layout can size this view to fit its content
    // exactly (used by the stacked mobile layout).
    readonly property alias contentHeight: content.implicitHeight

    Column {
        id: content
        anchors.centerIn: parent
        width: Math.min(parent.width - Theme.spacingLg * 2, 520)
        spacing: Theme.spacingLg
        visible: root.device !== null

        Rectangle {
            id: criticalBanner
            width: parent.width
            height: visible ? bannerLabel.implicitHeight + Theme.spacingMd * 2 : 0
            visible: root.device !== null && root.device.statusText === "critical"
            clip: true
            color: Theme.withAlpha(Theme.statusCritical, 0.15)
            border.color: Theme.statusCritical
            border.width: 1
            radius: Theme.radiusSm

            Behavior on height { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }

            Text {
                id: bannerLabel
                anchors.fill: parent
                anchors.margins: Theme.spacingSm
                text: qsTr("⚠ Critical temperature — immediate attention required")
                color: Theme.statusCritical
                font.bold: true
                font.pixelSize: Theme.fontSizeMd
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.device ? root.device.name : ""
            color: Theme.textPrimary
            font.pixelSize: Theme.fontSizeXl
            font.bold: true
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.spacingXl

            Gauge {
                value: root.device ? root.device.temperature / 100 : 0
                displayValue: root.device ? root.device.temperature : 0
                unit: " °C"
                progressColor: root.statusTint
            }

            Gauge {
                value: root.device ? root.device.signalStrength / 100 : 0
                displayValue: root.device ? root.device.signalStrength : 0
                unit: "%"
                progressColor: Theme.statusNormal
            }
        }

        StatusBadge {
            anchors.horizontalCenter: parent.horizontalCenter
            status: root.device ? root.device.statusText : "normal"
        }

        Text {
            text: qsTr("LIVE WAVEFORMS")
            color: Theme.textSecondary
            font.pixelSize: Theme.fontSizeSm
            font.bold: true
        }

        Column {
            width: parent.width
            spacing: Theme.spacingSm

            WaveformChart {
                width: parent.width
                channelLabel: qsTr("Pressure")
                unit: "cmH2O"
                samples: root.device ? root.device.pressureWaveform : []
                cursor: root.device ? root.device.waveformCursor : 0
                minValue: 0
                maxValue: 30
                lineColor: Theme.statusWarning
            }

            WaveformChart {
                width: parent.width
                channelLabel: qsTr("Flow")
                unit: "L/min"
                samples: root.device ? root.device.flowWaveform : []
                cursor: root.device ? root.device.waveformCursor : 0
                minValue: -60
                maxValue: 60
                lineColor: Theme.statusNormal
            }

            WaveformChart {
                width: parent.width
                channelLabel: qsTr("Volume")
                unit: "mL"
                samples: root.device ? root.device.volumeWaveform : []
                cursor: root.device ? root.device.waveformCursor : 0
                minValue: 0
                maxValue: 500
                lineColor: Theme.accent
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: root.device === null
        text: qsTr("Select a device from the list")
        color: Theme.textSecondary
        font.pixelSize: Theme.fontSizeLg
    }
}
