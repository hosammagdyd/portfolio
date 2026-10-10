qt_add_executable(tst_devicelistmodel
    tst_devicelistmodel.cpp
    ../core/device.h
    ../core/device.cpp
    ../core/devicelistmodel.h
    ../core/devicelistmodel.cpp
)

target_include_directories(tst_devicelistmodel PRIVATE ${CMAKE_SOURCE_DIR}/core)

target_link_libraries(tst_devicelistmodel PRIVATE Qt6::Test Qt6::Qml Qt6::StateMachine)

add_test(NAME DeviceListModelTest COMMAND tst_devicelistmodel)
