//  Copyright (c) 2018-2022 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/config.h>
#include <hpxc/util/init_hpx.hpp>

#include <hpx/hpx_init.hpp>

#include <string>
#include <vector>
#include <utility>

int hpxc_user_main_wrapper(int argc, char** argv)
{
    int retval = hpxc_user_main(argc, argv);

    hpx::finalize();
    return retval;
}

int main(int argc, char** argv)
{
    // allow for unknown options and inhibit aliasing of short options
    std::vector<std::string> const cfg = {
        "hpx.commandline.allow_unknown=1", "hpx.commandline.aliasing=0"};

    hpx::init_params init_args;
    init_args.cfg = std::move(cfg);

    return hpx::init(&hpxc_user_main_wrapper, argc, argv, init_args);
}
