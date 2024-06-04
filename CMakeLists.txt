cmake_minimum_required(VERSION 3.5)
project(monitors_demo)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_library(DXGI_LIBRARY dxgi)
if(NOT DXGI_LIBRARY)
    message(FATAL_ERROR "dxgi.lib not found")
endif()

file(GLOB MAIN_SOURCES "main.cpp")
add_executable(main ${MAIN_SOURCES})

file(GLOB MAIN_PDH_SOURCES "main_pdh.cpp")
add_executable(main_pdh ${MAIN_PDH_SOURCES})

file(GLOB MAIN_SYSTEM_TIMES_SOURCES "main_getsystemtimes.cpp")
add_executable(main_time ${MAIN_SYSTEM_TIMES_SOURCES})

add_subdirectory(monitors)

target_link_libraries(main_pdh PUBLIC pdh dxgi)
target_link_libraries(main_time PUBLIC monitors)
target_link_libraries(main PUBLIC monitors)

