import QtQuick
import QtShowcase

Rectangle {
    id: root
    property string status: "normal" // "normal" | "warning" | "critical"

    readonly property color tint: status === "critical" ? Theme.statusCritical
                                 : status === "warning" ? Theme.statusWarning
                                 : Theme.statusNormal

    implicitWidth: label.implicitWidth + Theme.spacingMd * 2
    implicitHeight: label.implicitHeight + Theme.spacingSm
    radius: height / 2
    color: Theme.withAlpha(tint, 0.16)
    border.color: Theme.withAlpha(tint, 0.4)
    border.width: 1
    scale: status === "critical" ? pulseScale : 1.0

    property real pulseScale: 1.0

    Behavior on color { ColorAnimation { duration: 200 } }
    Behavior on border.color { ColorAnimation { duration: 200 } }

    SequentialAnimation on pulseScale {
        running: root.status === "critical"
        loops: Animation.Infinite
        NumberAnimation { to: 1.1; duration: 450; easing.type: Easing.InOutQuad }
        NumberAnimation { to: 1.0; duration: 450; easing.type: Easing.InOutQuad }
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.status.toUpperCase()
        color: root.tint
        font.pixelSize: Theme.fontSizeSm
        font.bold: true
    }
}
