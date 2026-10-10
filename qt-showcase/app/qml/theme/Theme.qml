pragma Singleton
import QtQuick

QtObject {
    readonly property bool darkMode: Application.styleHints.colorScheme === Qt.Dark

    readonly property color background: darkMode ? "#121417" : "#f5f6f8"
    readonly property color surface: darkMode ? "#1c1f24" : "#ffffff"
    readonly property color surfaceAlt: darkMode ? "#262b33" : "#eef0f3"
    readonly property color textPrimary: darkMode ? "#f5f6f8" : "#1c1f24"
    readonly property color textSecondary: darkMode ? "#9aa3ad" : "#5b6470"
    readonly property color border: darkMode ? "#2d333b" : "#dde1e6"

    readonly property color statusNormal: "#2ecc71"
    readonly property color statusWarning: "#f1c40f"
    readonly property color statusCritical: "#e74c3c"
    readonly property color accent: "#3b82f6"

    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 16
    readonly property int spacingLg: 24
    readonly property int spacingXl: 32

    readonly property int radiusSm: 6
    readonly property int radiusMd: 12

    readonly property int fontSizeSm: 12
    readonly property int fontSizeMd: 14
    readonly property int fontSizeLg: 18
    readonly property int fontSizeXl: 24

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }
}
