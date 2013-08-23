//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#include <sys/time.h>
#endif

#define NTHREADS 100000

int counter = 0;

hpxc_mutex_t mut;
hpxc_cond_t cond;

void* incr(void* p)
{
    hpxc_mutex_lock(&mut);
    counter++;
    if(counter == NTHREADS)
        hpxc_cond_broadcast(&cond);
    hpxc_mutex_unlock(&mut);
    return NULL;
}

void *finish(void *p) {
    hpxc_mutex_lock(&mut);
    struct timespec tm;
    tm.tv_sec = 1;
    tm.tv_nsec = 0;
    while(counter < NTHREADS) {
        hpxc_cond_timedwait(&cond,&mut,tm);
    }
    hpxc_mutex_unlock(&mut);
    printf("counter=%d\n",counter);
    return NULL;
}

void my_init()
{
    int i;
    struct timeval tv;
    struct timezone tz;
    double t1,t2;
    hpxc_thread_attr_t attr;
    hpxc_thread_t fin;

    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t1 = tv.tv_sec + 1.0e-6*tv.tv_usec;

    hpxc_mutex_init(&mut,NULL);
    hpxc_cond_init(&cond,NULL);

    hpxc_thread_attr_init(&attr);
    hpxc_thread_attr_setdetachstate(&attr,HPXC_CREATE_DETACHED);
    for(i=0;i<NTHREADS;i++)
    {
        hpxc_thread_t th;
        hpxc_thread_create(&th,NULL,incr,NULL);
    }
    hpxc_thread_attr_destroy(&attr);

    hpxc_thread_create(&fin,NULL,finish,NULL);
    hpxc_thread_join(fin,NULL);

    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t2 = tv.tv_sec + 1.0e-6*tv.tv_usec;
    printf("time in seconds=%g\n",t2-t1);
}

int main(int argc, char* argv[]) {
    hpxc_init(my_init,argc,argv);
    return 0;
}
