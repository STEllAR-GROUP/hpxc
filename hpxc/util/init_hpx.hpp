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

// Dispatch differently depending on the number of arguments, such that hpxc_user_main always gets defined with two arguments
#define HPXC_MAIN0() hpxc_user_main(int _HPXC_UNUSED_ARGC, char** _HPXC_UNUSED_ARGV)
#define HPXC_MAIN1(...) hpxc_user_main(int _HPXC_UNUSED_ARGC, char** _HPXC_UNUSED_ARGV)
#define HPXC_MAIN2(...) hpxc_user_main(__VA_ARGS__)

#define HPXC_GET_MAIN_SIGNATURE(_1,_2, NAME,...) NAME
#define main(...) HPXC_GET_MAIN_SIGNATURE(__VA_ARGS__ __VA_OPT__(,) HPXC_MAIN2, HPXC_MAIN1, HPXC_MAIN0)(__VA_ARGS__)

#endif
