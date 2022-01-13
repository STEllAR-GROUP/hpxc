//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>

#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#include <sys/time.h>
#include <unistd.h>
#endif

hpxc_thread_attr_t attr;

void* fib(void* arg)
{
    hpxc_thread_t t1;
    hpxc_thread_t t2;
    size_t r1;
    size_t r2;

    size_t n = (size_t) arg;
    if (n < 2)
    {
        hpxc_thread_exit((void*) (n));
    }

    hpxc_thread_create(&t1, &attr, fib, (void*) (n - 1));
    hpxc_thread_create(&t2, &attr, fib, (void*) (n - 2));

    hpxc_thread_join(t1, (void*) &r1);
    hpxc_thread_join(t2, (void*) &r2);
    hpxc_thread_exit((void*) (r1 + r2));
    return NULL;
}

size_t fib_helper(size_t n)
{
    hpxc_thread_t thread;
    void* result;

    hpxc_thread_create(&thread, &attr, fib, (void*) n);
    hpxc_thread_join(thread, &result);
    return (size_t) (result);
}

int main(int argc, char* argv[])
{
    size_t n = 25;
    double t1, t2;
    struct timeval tv;
    struct timezone tz;
    size_t i;

    hpxc_thread_attr_init(&attr);
    timerclear(&tv);
    gettimeofday(&tv, &tz);
    t1 = tv.tv_sec + 1.0e-6 * tv.tv_usec;

    i = fib_helper(n);
    printf("Result: %zu\n", i);

    timerclear(&tv);
    gettimeofday(&tv, &tz);
    t2 = tv.tv_sec + 1.0e-6 * tv.tv_usec;
    printf("Elapsed: %f\n", t2 - t1);

    return 0;
}
