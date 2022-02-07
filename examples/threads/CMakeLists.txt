# Copyright (c) 2007-2022 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(example_programs
    attr_stacksize
    create_thread
    join_thread
    counter
    counter2
    counter3
    counter4
    self
    cancel
    thread_local_storage
    pthread_test
    fib_naive
    pthread_compatibility_header
)

foreach(example_program ${example_programs})
  set(sources ${example_program}.c)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_hpx_executable(
    ${example_program}
    SOURCES ${sources} ${${example_program}_FLAGS}
    DEPENDENCIES hpxc
    LANGUAGE C
    FOLDER "Examples/Threads"
  )

  set_target_properties(
    ${example_program} PROPERTIES COMPILE_DEFINITIONS HPXC_APPLICATION_EXPORTS
  )

  # add a custom target for this example
  add_hpx_pseudo_target(examples.threads.${example_program})

  # make pseudo-targets depend on master pseudo-target
  add_hpx_pseudo_dependencies(
    examples.threads examples.threads.${example_program}
  )

  # add dependencies to pseudo-target
  add_hpx_pseudo_dependencies(
    examples.threads.${example_program} ${example_program}_exe
  )
endforeach()
