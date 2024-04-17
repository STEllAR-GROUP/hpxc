//  Copyright (c) 2024 Panos Syskakis
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// if HPXC_HAVE_DYNAMIC_HPXC_MAIN is defined, the wrapping is accomplished by linking against hpxc_wrap.a (plus -wrap=main linker flag)
// else, it is done by the following macro, which renames main to hpxc_user_main
#if !defined(HPXC_EXPORTS) && !defined(HPXC_HAVE_DYNAMIC_HPXC_MAIN)

// Dispatch differently depending on the number of arguments, such that hpxc_user_main always gets defined with two arguments
#define HPXC_MAIN0() hpxc_user_main(int _HPXC_UNUSED_ARGC, char** _HPXC_UNUSED_ARGV)
#define HPXC_MAIN1(...) hpxc_user_main(int _HPXC_UNUSED_ARGC, char** _HPXC_UNUSED_ARGV)
#define HPXC_MAIN2(...) hpxc_user_main(__VA_ARGS__)

#define HPXC_GET_MAIN_SIGNATURE(_1,_2, NAME,...) NAME
#define main(...) HPXC_GET_MAIN_SIGNATURE(__VA_ARGS__ __VA_OPT__(,) HPXC_MAIN2, HPXC_MAIN1, HPXC_MAIN0)(__VA_ARGS__)

#endif
