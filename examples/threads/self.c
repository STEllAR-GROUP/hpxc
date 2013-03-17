//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

void* hello(void* p)
{
    printf("hello from id=%d\n",hpxc_thread_self());
    return 0;
}

#define NTHREADS 10
void my_init()
{
    int i;
    hpxc_thread_t threads[NTHREADS];
    for(i=0;i<NTHREADS;i++)
    {
        hpxc_thread_create(&threads[i],0,hello,0);
        printf(" was id=%d\n",threads[i]);
    }
    for(i=0;i<NTHREADS;i++)
    {
        hpxc_thread_join(threads[i],0);
    }
}

int main(int argc, char* argv[]) {
    hpxc_init(my_init,argc,argv);
    return 0;
}

