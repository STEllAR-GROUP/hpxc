//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPXC_CONFIG_CONFIG_SEP_19_2012_0221PM)
#define HPXC_CONFIG_CONFIG_SEP_19_2012_0221PM

#include <hpxc/config/export_definitions.h>

///////////////////////////////////////////////////////////////////////////////
#if defined(HPXC_APPLICATION_EXPORTS)
#  define main hpxc_user_main
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
extern "C" {
#endif

    HPXC_APPLICATION_EXPORT int hpxc_user_main();
     
#if defined(__cplusplus)
}
#endif

#endif

