//  Copyright (c) 2024 Panos Syskakis
//  Copyright (c) 2018-2022 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/util/init_hpx.hpp>

#if defined(HPXC_HAVE_DYNAMIC_HPXC_MAIN)

HPXC_EXTERN_C int __real_main(int argc, char** argv);
HPXC_EXTERN_C int initialize_main(int argc, char** argv);

HPXC_EXTERN_C int hpxc_user_main(int argc, char** argv){
    return __real_main(argc, argv);
}

HPXC_EXTERN_C int __wrap_main(int argc, char* argv[])
{
    return initialize_main(argc, argv);
}

#endif // HPXC_HAVE_DYNAMIC_HPXC_MAIN
