import QtQuick
import QtShowcase

Item {
    id: root
    clip: true
    property DeviceListModel model

    // When false, the list sizes itself to its full content height instead of
    // scrolling internally — used when an outer page-level Flickable should
    // own scrolling instead (the stacked mobile layout).
    property bool scrollableInternally: true
    readonly property alias contentHeight: listView.contentHeight

    signal deviceSelected(Device device)

    ListView {
        id: listView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: scrollableInternally ? parent.height : contentHeight
        interactive: scrollableInternally
        model: root.model
        spacing: Theme.spacingSm
        clip: true

        delegate: Card {
            id: card
            width: listView.width
            height: 72

            required property Device device

            MouseArea {
                anchors.fill: parent
                onClicked: root.deviceSelected(card.device)
            }

            Row {
                anchors.fill: parent
                anchors.margins: Theme.spacingSm
                spacing: Theme.spacingMd

                Column {
                    width: parent.width - statusBadge.width - Theme.spacingMd
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    Text {
                        text: card.device.name
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeMd
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }

                    Text {
                        text: card.device.temperature.toFixed(1) + " °C"
                        color: Theme.textSecondary
                        font.pixelSize: Theme.fontSizeSm
                    }
                }

                StatusBadge {
                    id: statusBadge
                    anchors.verticalCenter: parent.verticalCenter
                    status: card.device.statusText
                }
            }
        }
    }
}
