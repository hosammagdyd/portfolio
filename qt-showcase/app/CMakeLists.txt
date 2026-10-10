qt_add_executable(appQtShowcase
    main.cpp
)

target_include_directories(appQtShowcase PRIVATE
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/chart
)

set_source_files_properties(qml/theme/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(appQtShowcase
    URI QtShowcase
    VERSION 1.0
    SOURCES
        ../core/device.h
        ../core/device.cpp
        ../core/devicelistmodel.h
        ../core/devicelistmodel.cpp
        ../core/devicesimulator.h
        ../core/devicesimulator.cpp
        ../core/devicesimulatorworker.h
        ../core/devicesimulatorworker.cpp
        chart/waveformchartitem.h
        chart/waveformchartitem.cpp
    QML_FILES
        qml/Main.qml
        qml/theme/Theme.qml
        qml/components/Card.qml
        qml/components/StatusBadge.qml
        qml/components/Gauge.qml
        qml/components/WaveformChart.qml
        qml/views/DashboardView.qml
        qml/views/DeviceDetailView.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQtShowcase PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQtShowcase
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQtShowcase PRIVATE Qt6::Quick Qt6::StateMachine)

install(TARGETS appQtShowcase
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
