cmake_minimum_required(VERSION 3.10)
project(FTTH_Management CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES
    src/main.cpp
    src/Application.cpp
    src/use_cases/*.cpp
)

add_executable(ftth_app ${SOURCES})

set_target_properties(ftth_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
