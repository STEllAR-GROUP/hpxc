# Copyright (c) 2021-2022 Hartmut Kaiser
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(HPX_REQUIRED_HPX_VERSION 1.8.0)
set(HPXC_WITH_HPX_TAG_DEFAULT "1.8.0")

if(NOT HPXC_WITH_FETCH_HPX)
  find_package(HPX ${HPX_REQUIRED_HPX_VERSION} REQUIRED)
elseif(NOT TARGET HPX::hpx AND NOT HPX_FIND_PACKAGE)
  # Place HPX binaries in HPXC's binary directory by default when on Windows
  if(MSVC AND NOT DEFINED HPX_WITH_BINARY_DIR)
    set(HPX_WITH_BINARY_DIR
        "${PROJECT_BINARY_DIR}"
        CACHE
          STRING
          "Binary directory for HPX should be the same as for HPXC"
          FORCE
    )
  endif()

  if(FETCHCONTENT_SOURCE_DIR_HPX)
    hpxc_info(
      "HPXC_WITH_FETCH_HPX=${HPXC_WITH_FETCH_HPX}, HPX will be used through CMake's FetchContent and installed alongside HPXC (FETCHCONTENT_SOURCE_DIR_HPX=${FETCHCONTENT_SOURCE_DIR_HPX})"
    )
  else()
    hpxc_info(
      "HPXC_WITH_FETCH_HPX=${HPXC_WITH_FETCH_HPX}, HPX will be fetched using CMake's FetchContent and installed alongside HPXC (HPXC_WITH_HPX_TAG=${HPXC_WITH_HPX_TAG})"
    )
  endif()
  include(FetchContent)

  set(HPXC_WITH_HPX_TAG
      "${HPXC_WITH_HPX_TAG_DEFAULT}"
      CACHE STRING "HPX repository tag or branch" FORCE
  )
  fetchcontent_declare(
    HPX
    GIT_REPOSITORY https://github.com/STEllAR-GROUP/hpx.git
    GIT_TAG ${HPXC_WITH_HPX_TAG}
  )

  fetchcontent_makeavailable(HPX)

  # make sure the HPXLocal cmake files are available to HPX
  list(APPEND CMAKE_MODULE_PATH "${hpx_SOURCE_DIR}/cmake")

endif()
