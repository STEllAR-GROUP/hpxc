//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/pthread_compatibility.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#  include <unistd.h>
#  include <sys/time.h>
#endif

pthread_attr_t attr;

void* fib(void* arg){
    long n=(long)arg;
    pthread_t t1;
    pthread_t t2;
    void* r1;
    void* r2;

    if(n==1 || n==2){
        pthread_exit( (void*)(1));
    }
    pthread_create(&t1, &attr, fib, (void*)(n-1));
    pthread_create(&t2, &attr, fib, (void*)(n-2));

    pthread_join(t1,&r1);
    pthread_join(t2,&r2);

    pthread_exit((void*)((long)(r1)+(long)(r2)));
    return NULL;
}

long fib_helper(long n){
    pthread_t thread;
    void* result;

    pthread_create(&thread, &attr, fib, (void*)(n));
    pthread_join(thread, &result);
    return (long)(result);
}

int hpxc_main(){
    long n=13;

    double t1,t2;
    struct timeval tv;
    struct timezone tz;
    long i;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t1= tv.tv_sec + 1.0e-6*tv.tv_usec;

    i=fib_helper(n);
    printf("Result: %ld\n",i);

    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t2= tv.tv_sec + 1.0e-6*tv.tv_usec;
    printf("Elapsed: %f\n",t2-t1);

    return 0;
}
