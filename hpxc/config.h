//  Copyright (c) 2007-2022 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpxc/config/config_definitions.h>
#include <hpxc/config/export_definitions.h>

#if !defined(_MSC_VER)
//#define HPXC_HAVE_RW_LOCK
#endif

#if !defined(HPXC_SMALL_STACK_SIZE)
#define HPXC_SMALL_STACK_SIZE 0x8000    // 32kByte
#endif
