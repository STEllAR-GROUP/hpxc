# Copyright (c) 2024 Panos Syskakis
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

list(TRANSFORM hpxc_headers PREPEND ${PROJECT_SOURCE_DIR}/ OUTPUT_VARIABLE hpxc_headers)
list(TRANSFORM hpxc_sources PREPEND ${PROJECT_SOURCE_DIR}/ OUTPUT_VARIABLE hpxc_sources)

add_hpx_library(
  hpxc_wrap STATIC
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}"
  SOURCES ${PROJECT_SOURCE_DIR}/src/util/wrap_main.cpp
  HEADERS ${PROJECT_SOURCE_DIR}/hpxc/util/wrap_main.h
)
target_include_directories(hpxc_wrap PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)
if(HPXC_WITH_DYNAMIC_HPXC_MAIN)
  hpxc_add_config_definition(HPXC_HAVE_DYNAMIC_HPXC_MAIN)
  target_link_options(hpxc_wrap PUBLIC -Wl,-wrap=main)
endif()
add_library(HPXC::hpxc_wrap ALIAS hpxc_wrap)

set(config_header "${CMAKE_BINARY_DIR}/hpxc/config/config_definitions.h")
write_config_definitions_file(${config_header})
set(hpxc_headers ${hpxc_headers} ${config_header})

# Compile as static or shared library
if(HPXC_WITH_SHARED_LIBS)
  set(HPXC_LIBRARY_TYPE SHARED)
else()
  set(HPXC_LIBRARY_TYPE STATIC)
endif()

add_hpx_library(
  hpxc "${HPXC_LIBRARY_TYPE}"
  SOURCE_ROOT "${PROJECT_SOURCE_DIR}"
  HEADER_ROOT "${PROJECT_SOURCE_DIR}"
  HEADERS "${hpxc_headers}"
  SOURCES "${hpxc_sources}"
  DEPENDENCIES HPX::hpx
)
target_include_directories(hpxc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
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
add_library(HPXC::hpxc ALIAS hpxc)

# Install the hpxc library
install(TARGETS hpxc hpxc_wrap
  EXPORT HPXCTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)
