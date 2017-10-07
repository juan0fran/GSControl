cmake_minimum_required(VERSION 2.6)

project(SocketUsage)
cmake_policy(SET CMP0042 NEW)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/orbit_propagator/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(/usr/include)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/apps/${CMAKE_BUILD_TYPE})
MARK_AS_ADVANCED(
        LIBRARY_OUTPUT_PATH
        EXECUTABLE_OUTPUT_PATH
)

link_directories(${LIBRARY_OUTPUT_PATH})
link_directories(/usr/lib/)
link_directories(/usr/local/lib/)

add_subdirectory(src)
add_subdirectory(apps)
