# Copyright (C) 2025 Igal Alkon and ALKONTEK
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.21)
project(oxide VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(oxide INTERFACE)

target_include_directories(oxide INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(oxide INTERFACE cxx_std_23)

# Define installation directory for headers
set(INCLUDE_INSTALL_DIR include)

# Create examples executable and link it to oxide
add_executable(oxide_examples examples.cpp)
target_link_libraries(oxide_examples oxide)

install(TARGETS oxide
    EXPORT oxideTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES oxide.hpp DESTINATION include)

install(EXPORT oxideTargets
    FILE oxideTargets.cmake
    NAMESPACE oxide::
    DESTINATION lib/cmake/oxide
)

### EXPORT #####################################################################

# Create a package config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    oxideConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/oxideConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/oxideConfig.cmake"
    INSTALL_DESTINATION lib/cmake/oxide
    PATH_VARS
        INCLUDE_INSTALL_DIR
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/oxideConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/oxideConfigVersion.cmake"
    DESTINATION lib/cmake/oxide
)
