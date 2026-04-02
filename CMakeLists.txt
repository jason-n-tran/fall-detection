cmake_minimum_required(VERSION 3.16)
project(FallDetector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(FallDetector
    main.cpp
)

target_link_libraries(FallDetector PRIVATE)