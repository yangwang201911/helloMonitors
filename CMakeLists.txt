cmake_minimum_required(VERSION 3.5)
project(monitors_demo)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(OpenCV REQUIRED COMPONENTS core highgui videoio imgproc imgcodecs)

file(GLOB SOURCES "*.cpp")
add_executable(test ${SOURCES})
add_subdirectory(monitors)

target_link_libraries(test PUBLIC monitors)

