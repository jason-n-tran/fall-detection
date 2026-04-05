cmake_minimum_required(VERSION 3.16)
project(FallDetector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick Core Gui REQUIRED)

add_executable(FallDetector
    main.cpp
    imusensor.cpp
    imusensor.h
    matrix.h
    orientation_filter.h
    circular_buffer.h
    resources.qrc
)

target_link_libraries(FallDetector PRIVATE Qt6::Core Qt6::Gui Qt6::Quick)