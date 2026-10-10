import QtQuick
import QtShowcase

Rectangle {
    id: root
    default property alias content: contentItem.children

    clip: true
    color: Theme.surface
    radius: Theme.radiusMd
    border.color: Theme.border
    border.width: 1

    Item {
        id: contentItem
        anchors.fill: parent
    }
}
