//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/config/export_definitions.h>

// #define HPX_MAIN_EXPORT HPXC_API_EXPORT

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <boost/function.hpp>
#include <boost/plugin.hpp>

///////////////////////////////////////////////////////////////////////////////
std::string get_executable_filename(int argc, char* argv[]);

///////////////////////////////////////////////////////////////////////////////
// Default implementation of main() if all the user provides is hpx::user_main.
HPXC_SYMBOL_EXPORT int main(int argc, char *argv[])
{
    return hpx::init(argc, argv);
}

///////////////////////////////////////////////////////////////////////////////
// Forwarding of hpx_main, if necessary. This has to be in a separate
// translation unit to ensure the linker can pick or ignore this function,
// depending on whether the main executable defines this symbol or not.
int hpx_main(int argc, char* argv[])
{
    typedef int (*function_type)(int, char*[]);
    typedef boost::function<void(function_type)> deleter_type;

    // Bind the hpxc_user_main symbol dynamically and invoke it
    boost::plugin::dll this_exe(get_executable_filename(argc, argv));
    std::pair<function_type, deleter_type> p = 
        this_exe.get<function_type, deleter_type>("hpxc_user_main");

    int result = (*p.first)(argc, argv);

    hpx::finalize();
    return result;
}

