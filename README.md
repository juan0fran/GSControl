## Socket Utils example

Use socket library in /src to create your programs.

Programs shall be created in /apps folder as main.cpp

In order to add a new program to use the library, add it to CMakeLists on apps folder as:

```
file (GLOB EXECUTABLE_NAME SOURCE_NAME.cpp)

set(EXAMPLE_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/sensor_core CACHE STRING "EXECUTABLE_NAME dir")

add_executable(EXECUTABLE_NAME ${EXECUTABLE_NAME})

target_link_libraries(EXECUTABLE_NAME utils)
```

Substituting EXECUTABLE_NAME by your program name and SOURCE_NAME by your .cpp file...
