//  Copyright (c) 2022 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if defined(__cplusplus)
extern "C" {
#endif

int hpxc_user_main(int argc, char** argv);

#if defined(__cplusplus)
}
#endif

#if !defined(HPXC_EXPORTS)
#define main(...) hpxc_user_main(__VA_ARGS__)
#endif
