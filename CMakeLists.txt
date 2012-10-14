# Copyright (c) 2007-2012 Hartmut Kaiser
# Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying 
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

################################################################################
# Top level CMakeList.txt for HPXC
################################################################################
  
cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

project(hpxc CXX C)

# Project version.
set(HPXC_MAJOR_VERSION 0)
set(HPXC_MINOR_VERSION 1)
set(HPXC_PATCH_LEVEL   0)
set(HPXC_SOVERSION     ${HPXC_MAJOR_VERSION})
set(HPXC_VERSION
    "${HPXC_MAJOR_VERSION}.${HPXC_MINOR_VERSION}.${HPXC_PATCH_LEVEL}")

set(CMAKE_MODULE_PATH
    ${HPX_ROOT}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules)

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

find_package(HPX)

if(NOT MSVC)
  set(HPX_RPATH
      "${HPX_RPATH}:${CMAKE_INSTALL_PREFIX}/lib/hpx:${CMAKE_BINARY_DIR}/lib/hpx")
endif()

include_directories(${HPX_INCLUDE_DIR})
link_directories(${HPX_LIBRARY_DIR})

include_directories(${hpxc_SOURCE_DIR})

add_subdirectory(src)

##############################################################################
add_hpx_pseudo_target(examples)
add_subdirectory(examples)

