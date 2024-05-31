# Copyright (C) 2018-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set(SOURCES
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/presenter.cpp)

set(HEADERS
    include/monitors/cpu_monitor.h
    include/monitors/memory_monitor.h
    include/monitors/presenter.h)

if(WIN32)
    list(APPEND SOURCES src/query_wrapper.cpp src/query_wrapper.h)
endif()

if(OpenCV_FOUND)
    # Add OpenCV header files top project
    message(STATUS "OpenCV dir: ${OpenCV_INCLUDE_DIRS}")
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

add_library(monitors STATIC ${SOURCES} ${HEADERS} ${OpenCV_INCLUDE_DIRS})
target_include_directories(monitors PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(monitors PRIVATE ${OpenCV_LIBS})
if(WIN32)
    target_link_libraries(monitors PRIVATE pdh)
endif()
