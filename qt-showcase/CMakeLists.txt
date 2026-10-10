cmake_minimum_required(VERSION 3.16)

project(QtShowcase VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml Test StateMachine)

qt_standard_project_setup(REQUIRES 6.8)

enable_testing()

add_subdirectory(app)
add_subdirectory(tests)
