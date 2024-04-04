//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <hpxc/util/wrap_main.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

void* hello(void* p)
{
    printf("hello from id=%p\n", hpxc_thread_self().handle);
    return 0;
}

#define NTHREADS 10
int main(int argc, char* argv[])
{
    int i;
    hpxc_thread_t threads[NTHREADS];
    for (i = 0; i < NTHREADS; i++)
    {
        hpxc_thread_create(&threads[i], 0, hello, 0);
        printf(" was id=%z\n", threads[i].handle);
    }
    printf("Comparing threads[0] and threads[1]: %d\n",
        hpxc_thread_equal(threads[0], threads[1]));
    printf("Comparing threads[0] and threads[0]: %d\n",
        hpxc_thread_equal(threads[0], threads[0]));
    for (i = 0; i < NTHREADS; i++)
    {
        hpxc_thread_join(threads[i], 0);
    }

    return 0;
}
