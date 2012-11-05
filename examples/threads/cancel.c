//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int counter = 0;

hpxc_mutex_t mut;

void* until_I_die(void* p)
{
    while(1)
        hpxc_thread_testcancel();
	return 0;
}

void my_init()
{
	int i;
	hpxc_thread_t once;
	hpxc_thread_create(&once,NULL,until_I_die,NULL);
    hpxc_thread_cancel(once);
	hpxc_thread_join(once,NULL);
}

int main(int argc, char* argv[]) {
	hpxc_init(my_init,argc,argv);
	return 0;
}

