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
project(oxide VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(oxide INTERFACE)

target_include_directories(oxide INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(oxide INTERFACE cxx_std_23)

if(MSVC)
    target_compile_options(oxide INTERFACE /EHsc)
endif()

target_compile_definitions(oxide INTERFACE
        OXIDE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        OXIDE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        OXIDE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        OXIDE_VERSION="${PROJECT_VERSION}"
)

# Define installation directory for headers
set(INCLUDE_INSTALL_DIR include)

# Match example
add_executable(oxide_match_example examples/match.cpp)
target_link_libraries(oxide_match_example oxide)

# Functional example
add_executable(oxide_functional_example examples/functional.cpp)
target_link_libraries(oxide_functional_example oxide)

# Covariant dispatch example
add_executable(oxide_covariant_dispatch_example examples/covariant_dispatch.cpp)
target_link_libraries(oxide_covariant_dispatch_example oxide)

# Vector example
add_executable(oxide_vector_example examples/vector.cpp)
target_link_libraries(oxide_vector_example oxide)

# Database example
add_executable(oxide_database_example examples/database.cpp)
target_link_libraries(oxide_database_example oxide)

install(TARGETS oxide
        EXPORT oxideTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

install(FILES include/oxide.hpp DESTINATION include)

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
