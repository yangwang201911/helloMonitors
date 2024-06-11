# Copyright (C) 2018-2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/monitors/*.h)

if(NOT WIN32)
    list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/query_wrapper.cpp)
    list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/monitors/query_wrapper.h)
endif()

add_library(monitors STATIC ${SOURCES} ${HEADERS})
target_include_directories(monitors PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(WIN32)
    target_link_libraries(monitors PRIVATE pdh dxgi)
endif()
