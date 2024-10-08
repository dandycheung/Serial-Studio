#
# Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

#-------------------------------------------------------------------------------
# Project setup
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.19)
project(qtcsv CXX)

#-------------------------------------------------------------------------------
# C++ options
#-------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Add external dependencies
#-------------------------------------------------------------------------------

find_package(Qt6 COMPONENTS Core REQUIRED)

#-------------------------------------------------------------------------------
# Import source code & resources
#-------------------------------------------------------------------------------

include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
 sources/writer.cpp
 sources/reader.cpp
 sources/variantdata.cpp
 sources/stringdata.cpp
 sources/contentiterator.cpp
)

set(HEADERS
 sources/symbols.h
 sources/filechecker.h
 sources/contentiterator.h

 include/qtcsv/writer.h
 include/qtcsv/reader.h
 include/qtcsv/stringdata.h
 include/qtcsv/variantdata.h
 include/qtcsv/qtcsv_global.h
 include/qtcsv/abstractdata.h
)

#-------------------------------------------------------------------------------
# Compile & link the library
#-------------------------------------------------------------------------------

add_library(
 qtcsv
 STATIC
 ${SOURCES}
 ${HEADERS}
)

target_link_libraries(
 qtcsv PUBLIC
 Qt6::Core
)

target_include_directories(
 qtcsv PUBLIC
 ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#-------------------------------------------------------------------------------
# Configure unity build
#-------------------------------------------------------------------------------

set_target_properties(
 qtcsv PROPERTIES
 UNITY_BUILD ON
 UNITY_BUILD_MODE BATCH
 UNITY_BUILD_BATCH_SIZE 128
 INTERPROCEDURAL_OPTIMIZATION TRUE
)
