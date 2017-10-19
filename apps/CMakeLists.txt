file (GLOB example main.cpp)
set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "example dir")
add_executable(example ${example})
target_link_libraries(example external_control orbit_simulator)

file (GLOB rotor rotor.cpp)
set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "rotor dir")
add_executable(rotor ${rotor})
target_link_libraries(rotor external_control orbit_simulator)

file (GLOB control control.cpp)
set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "control dir")
add_executable(control ${control})
target_link_libraries(control gs_control external_control orbit_simulator)

file (GLOB sender socket_sender.cpp)
set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "sender dir")
add_executable(sender ${sender})
target_link_libraries(sender gs_control external_control orbit_simulator)

file (GLOB freq_tune set_initial_freq.cpp)
set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "freq_tune dir")
add_executable(freq_tune ${freq_tune})
target_link_libraries(freq_tune external_control)
