# Copyright (c) 2024 Panos Syskakis
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

function(hpxc_add_config_definition definition)
set(argn ${ARGN})
  list(LENGTH argn ArgsLen)
  if(ArgsLen GREATER 0)
    set_property(
      GLOBAL APPEND PROPERTY HPXC_CONFIG_DEFINITIONS "${definition} ${ARGN}"
    )
  else()
    set_property(GLOBAL APPEND PROPERTY HPXC_CONFIG_DEFINITIONS "${definition}")
  endif()
endfunction()

function(write_config_definitions_file filename)
    get_property(DEFINITIONS_VAR GLOBAL PROPERTY HPXC_CONFIG_DEFINITIONS)
    set(hpxc_config_defines "\n")
    foreach(def ${DEFINITIONS_VAR})
        set(hpxc_config_defines "${hpxc_config_defines}#define ${def}\n")
    endforeach()
    file(WRITE _${filename}_TEMP "${hpxc_config_defines} \n")
    configure_file(_${filename}_TEMP ${filename} COPYONLY)
    file(REMOVE _${filename}_TEMP)
endfunction()
