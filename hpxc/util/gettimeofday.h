//  Copyright (c) 2007-2013 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPXC_GETTIMEOFDAY_MAR_16_2013_0844PM)
#define HPXC_GETTIMEOFDAY_MAR_16_2013_0844PM

#if defined(_MSC_VER)

#include <hpxc/config/export_definitions.h>
#include <time.h>
#include <WinSock2.h>
#include <windows.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct timezone
{
    __int32  tz_minuteswest;    /* minutes W of Greenwich */
    BOOL  tz_dsttime;           /* type of dst correction */
};

HPXC_API_EXPORT int gettimeofday(struct timeval *tv, struct timezone *tz);

#if defined(__cplusplus)
}
#endif

#endif

#endif

