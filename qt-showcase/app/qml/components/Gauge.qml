import QtQuick
import QtShowcase

Item {
    id: root
    property real value: 0        // normalized 0..1
    property real displayValue: 0 // raw number shown as text
    property string unit: ""
    property color trackColor: Theme.surfaceAlt
    property color progressColor: Theme.accent

    implicitWidth: 140
    implicitHeight: 140

    readonly property real clampedValue: Math.max(0, Math.min(1, value))

    Behavior on value {
        NumberAnimation { duration: 400; easing.type: Easing.OutCubic }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()

            const w = width
            const h = height
            const cx = w / 2
            const cy = h / 2
            const r = Math.min(w, h) / 2 - 10
            const startAngle = Math.PI * 0.75
            const endAngle = Math.PI * 2.25

            ctx.lineWidth = 10
            ctx.lineCap = "round"

            ctx.strokeStyle = root.trackColor
            ctx.beginPath()
            ctx.arc(cx, cy, r, startAngle, endAngle)
            ctx.stroke()

            ctx.strokeStyle = root.progressColor
            ctx.beginPath()
            ctx.arc(cx, cy, r, startAngle, startAngle + (endAngle - startAngle) * root.clampedValue)
            ctx.stroke()
        }
    }

    onValueChanged: canvas.requestPaint()
    onTrackColorChanged: canvas.requestPaint()
    onProgressColorChanged: canvas.requestPaint()
    Component.onCompleted: canvas.requestPaint()

    Text {
        anchors.centerIn: parent
        text: root.displayValue.toFixed(1) + root.unit
        color: Theme.textPrimary
        font.pixelSize: Theme.fontSizeXl
        font.bold: true
    }
}
