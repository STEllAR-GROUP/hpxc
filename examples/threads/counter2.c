//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NTHREADS 1000000

int counter = 0;

hpxc_mutex_t mut;// = HPXC_MUTEX_INITIALIZER;
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
	while(counter < NTHREADS) {
		hpxc_cond_wait(&cond,&mut);
	}
	hpxc_mutex_unlock(&mut);
	printf("counter=%d\n",counter);
	return NULL;
}

void my_launch()
{
	int i;
	struct timeval tv;
	struct timezone tz;
	double t1,t2;
	mut = HPXC_MUTEX_INITIALIZER;

	timerclear(&tv);
	gettimeofday(&tv,&tz);
	t1 = tv.tv_sec + 1.0e-6*tv.tv_usec;

	hpxc_mutex_init(&mut,NULL);
	hpxc_cond_init(&cond,NULL);
	for(i=0;i<NTHREADS;i++)
	{
		hpxc_thread_t th;
		hpxc_thread_create(&th,NULL,incr,NULL);
		hpxc_thread_detach(th);
	}
	hpxc_thread_t fin;
	hpxc_thread_create(&fin,NULL,finish,NULL);
	hpxc_thread_join(fin,NULL);

	timerclear(&tv);
	gettimeofday(&tv,&tz);
	t2 = tv.tv_sec + 1.0e-6*tv.tv_usec;
	printf("time in seconds=%g\n",t2-t1);
}

#undef main
int main(int argc, char* argv[]) {
	hpxc_launch(argc,argv,my_launch);
	return 0;
}
