# Copyright (c) 2007-2022 Hartmut Kaiser Copyright (c) 2011-2012 Bryce
# Adelstein-Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# ##############################################################################
# Top level CMakeList.txt for HPXC
# ##############################################################################

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(hpxc CXX C)

# Project version.
set(HPXC_MAJOR_VERSION 0)
set(HPXC_MINOR_VERSION 2)
set(HPXC_PATCH_LEVEL 0)
set(HPXC_SOVERSION ${HPXC_MAJOR_VERSION})
set(HPXC_VERSION
    "${HPXC_MAJOR_VERSION}.${HPXC_MINOR_VERSION}.${HPXC_PATCH_LEVEL}"
)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

cmake_policy(SET CMP0022 NEW)

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

include(HPXC_Utils)
include(HPXC_SetOutputPaths)
include(HPXC_SetupHPX)

option(HPXC_WITH_DYNAMIC_HPXC_MAIN "Build hpxc with dynamic main wrap" ON)
option(HPXC_WITH_SHARED_LIBS "Build hpxc as a shared library" OFF)

add_subdirectory(src)

# ##############################################################################
add_hpx_pseudo_target(examples)
add_subdirectory(examples)

message(${CMAKE_BINARY_DIR})

install(
  DIRECTORY hpxc
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "*.hpp"
)
