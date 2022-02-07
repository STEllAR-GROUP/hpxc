# Copyright (c) 2007-2022 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(hpxc_headers
    hpxc/config.h
    hpxc/config/export_definitions.h
    hpxc/threads/readers_writers_mutex.hpp
    hpxc/threads/thread.h
    hpxc/util/init_hpx.hpp
    hpxc/util/gettimeofday.h
    hpxc/threads.h
    hpxc/pthread_compatibility.h
)

set(hpxc_sources src/threads/thread.cpp src/util/gettimeofday.cpp
                 src/util/init_hpx.cpp
)

add_hpx_library(
  hpxc STATIC
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}"
  HEADERS ${hpxc_headers}
  SOURCES ${hpxc_sources}
  DEPENDENCIES HPX::hpx
  PREPEND_HEADER_ROOT PREPEND_SOURCE_ROOT ESSENTIAL
)

set_target_properties(
  hpxc
  PROPERTIES COMPILE_DEFINITIONS HPXC_EXPORTS
             VERSION ${HPXC_VERSION}
             SOVERSION ${HPXC_SOVERSION}
             CLEAN_DIRECT_OUTPUT 1
             OUTPUT_NAME hpxc${HPX_DEBUG_POSTFIX}
             FOLDER "Core"
)

install(TARGETS hpxc DESTINATION lib)
