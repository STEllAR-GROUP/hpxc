# Copyright (c) 2007-2012 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(example_programs
    create_thread
    join_thread
	counter
	counter2
	counter3
    self
    cancel
    many_thread_test
   )

add_definitions("-DHPXC_APPLICATION_EXPORTS")

foreach(example_program ${example_programs})
  set(sources ${example_program}.c)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_hpx_executable(${example_program}
                     SOURCES ${sources}
                     ${${example_program}_FLAGS}
                     DEPENDENCIES hpxc_lib
                     LANGUAGE C
                     FOLDER "Examples/Threads/${example_program}"
                     OUTPUT_SUFFIX bin
                     NOHPXINIT)

  # add a custom target for this example
  add_hpx_pseudo_target(examples.threads.${example_program})

  # make pseudo-targets depend on master pseudo-target
  add_hpx_pseudo_dependencies(examples.threads
                              examples.threads.${example_program})

  # add dependencies to pseudo-target
  add_hpx_pseudo_dependencies(examples.threads.${example_program}
                              ${example_program}_exe)
endforeach()

