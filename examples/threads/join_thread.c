//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>

void* hello_thread(void* p)
{
    printf("hello");
    hpxc_thread_exit(NULL);
    return NULL; 
}

int main(int argc, char** argv)
{
    hpxc_thread_t thread;
    hpxc_thread_create(&thread, NULL, hello_thread, NULL); 

    hpxc_thread_join(thread, NULL);

    printf(" world\n");

    hpxc_thread_exit(NULL);
    return 0;
}
