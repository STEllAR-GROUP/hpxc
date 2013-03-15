#include <hpxc/threads.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

hpxc_thread_attr_t attr;

void* fib(void* arg){
    long n=(long)arg;
    if(n==1 || n==2){
        hpxc_thread_exit( (void*)(1));
    }
    hpxc_thread_t t1;
    hpxc_thread_t t2;
    hpxc_thread_create(&t1, &attr, fib, (void*)(n-1));
    hpxc_thread_create(&t2, &attr, fib, (void*)(n-2));
    void* r1;
    void* r2;
    hpxc_thread_join(t1,&r1);
    hpxc_thread_join(t2,&r2);
    hpxc_thread_exit((void*)((long)(r1)+(long)(r2)));
    return NULL;
}

long fib_helper(long n){
    hpxc_thread_t thread;
    hpxc_thread_create(&thread, &attr, fib, (void*)(n));
    void* result;
    hpxc_thread_join(thread, &result);
    return (long)(result);
}

int hpxc_main(){
    long n=20;
    hpxc_thread_attr_init(&attr);

    double t1,t2;
    struct timeval tv;
    struct timezone tz;
    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t1= tv.tv_sec + 1.0e-6*tv.tv_usec;

    long i=fib_helper(n);
    printf("Result: %d\n",i);
    
    timerclear(&tv);
    gettimeofday(&tv,&tz);
    t2= tv.tv_sec + 1.0e-6*tv.tv_usec;
    printf("Elapsed: %f\n",t2-t1);

    return 0;
}
