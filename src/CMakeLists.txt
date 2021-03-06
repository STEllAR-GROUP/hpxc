# Copyright (c) 2007-2012 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include_directories(${HPX_INCLUDE_DIR})
link_directories(${HPX_LIBRARY_DIR})

#add_hpx_library_sources(hpxc
#  GLOB_RECURSE GLOBS "${hpxc_SOURCE_DIR}/src/*.cpp")
#
#add_hpx_source_group(
#  NAME hpxc
#  CLASS "Source Files"
#  ROOT ${hpxc_SOURCE_DIR}/src
#  TARGETS ${hpxc_SOURCES})
#
#add_hpx_library_headers(hpxc
#  GLOB_RECURSE GLOBS "${hpxc_SOURCE_DIR}/hpxc/*.hpp")
#
#add_hpx_source_group(
#  NAME hpxc
#  CLASS "Header Files"
#  ROOT ${hpxc_SOURCE_DIR}/hpxc
#  TARGETS ${hpxc_HEADERS})

add_definitions("-DHPXC_EXPORTS")

# We intentionally check HPX_BUILD_TYPE, not the application build type, as
# the HPX libraries will be suffixed depending on their build type, not ours.
add_hpx_library(hpxc SHARED
    SOURCE_ROOT "${hpxc_SOURCE_DIR}/src"
    HEADER_ROOT "${hpxc_SOURCE_DIR}/hpxc"
    DEPENDENCIES hpx_init${HPX_DEBUG_POSTFIX}
    OUTPUT_SUFFIX lib/hpx
    AUTOGLOB
    ESSENTIAL)

# Don't cause our applications to link against hpx_init.
set_target_properties(hpxc_lib PROPERTIES LINK_INTERFACE_LIBRARIES "")

set_target_properties(hpxc_lib PROPERTIES
  VERSION ${HPXC_VERSION}
  SOVERSION ${HPXC_SOVERSION}
  CLEAN_DIRECT_OUTPUT 1
  OUTPUT_NAME hpxc${HPX_DEBUG_POSTFIX}
  FOLDER "Core")
install(TARGETS hpxc_lib
    DESTINATION lib)
