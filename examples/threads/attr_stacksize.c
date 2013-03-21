//  Copyright (c) 2012-2013 Alexander Duchene
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

int hpxc_main(int argc, char** argv)
{
    hpxc_thread_attr_t attr;
    hpxc_thread_attr_init(&attr);
    hpxc_thread_attr_setstacksize(&attr,0x10000);
    hpxc_thread_t thread;
    hpxc_thread_create(&thread, &attr, hello_thread, NULL); 

    hpxc_thread_join(thread, NULL);

    printf(" world\n");

    return 0;
}
