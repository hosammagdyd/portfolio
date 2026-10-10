import QtQuick
import QtShowcase

Item {
    id: root
    property var samples: []
    property int cursor: 0
    property real minValue: 0
    property real maxValue: 100
    property string unit: ""
    property string channelLabel: ""
    property color lineColor: Theme.accent
    property color gridColor: Theme.border
    property int gridRows: 4

    implicitHeight: 100

    Rectangle {
        anchors.fill: parent
        color: Theme.surfaceAlt
        radius: Theme.radiusSm
    }

    Item {
        id: axisLabels
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingXs
        width: 28

        Repeater {
            model: root.gridRows + 1

            delegate: Text {
                required property int index

                width: axisLabels.width
                horizontalAlignment: Text.AlignRight
                y: (axisLabels.height / root.gridRows) * index - implicitHeight / 2
                text: (root.maxValue - ((root.maxValue - root.minValue) / root.gridRows) * index).toFixed(0)
                color: Theme.textSecondary
                font.pixelSize: 9
            }
        }
    }

    // GPU-rendered via the scene graph (WaveformChartItem, C++) — see app/chart/.
    WaveformChartItem {
        anchors.left: axisLabels.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingXs

        samples: root.samples
        cursor: root.cursor
        minValue: root.minValue
        maxValue: root.maxValue
        lineColor: root.lineColor
        gridColor: root.gridColor
        gridRows: root.gridRows
    }

    Text {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: Theme.spacingXs
        text: root.channelLabel + (root.unit ? (" (" + root.unit + ")") : "")
        color: Theme.textSecondary
        font.pixelSize: Theme.fontSizeSm
        font.bold: true
    }
}
