//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include <boost/function.hpp>
#include <hpx/util/plugin.hpp>

///////////////////////////////////////////////////////////////////////////////
std::string get_executable_filename();

///////////////////////////////////////////////////////////////////////////////
// Default implementation of main() if all the user provides is hpx::user_main.

HPXC_SYMBOL_EXPORT int main(int argc, char *argv[])
{
    if (1 <= argc)
        return hpx::init(argv[0], argc, argv);
    else
        return hpx::init(argc, argv);
}

void (*user_init_func)()=0;

int hpx_init(boost::program_options::variables_map& vm) {
	user_init_func();
    return hpx::finalize();
}

extern "C" void hpxc_init(void (*init_func)(),int argc,char *argv[])
{
	user_init_func = init_func;
	hpx::init(hpx_init,argv[0],argc,argv);
}


// IMPLEMENT: Support for the main(void) signature.
int hpx_main(int argc, char* argv[])
{
    {
        typedef int (*function_type)(int, char*[]);
        typedef boost::function<void(function_type)> deleter_type;

        // Bind the hpxc_main symbol dynamically and invoke it.
        hpx::util::plugin::dll this_exe(get_executable_filename());
        std::pair<function_type, deleter_type> p = 
            this_exe.get<function_type, deleter_type>("hpxc_main");

		HPX_STD_BIND(*p.first,argc,argv)();
    }

    return hpx::finalize();
}

