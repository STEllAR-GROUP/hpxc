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

void* incr(void* p)
{
	hpxc_mutex_lock(&mut);
	counter++;
	hpxc_mutex_unlock(&mut);
	return 0;
}

#define NTHREADS 100000
void my_launch()
{
	int i;
	hpxc_mutex_init(&mut,0);
	hpxc_thread_t threads[NTHREADS];
	for(i=0;i<NTHREADS;i++)
	{
		hpxc_thread_create(&threads[i],0,incr,0);
	}
	for(i=0;i<NTHREADS;i++)
	{
		hpxc_thread_join(threads[i],0);
	}
	printf("counter=%d\n",counter);
}

extern void hpxc_launch(int argc,char *argv[],void (*launch)());

#undef main
int main(int argc, char* argv[]) {
	hpxc_launch(argc,argv,my_launch);
	return 0;
}

