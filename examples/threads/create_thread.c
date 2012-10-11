//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* hello_thread(void* p)
{
	int *n = (int *)p;
    printf("hello world=%d\n",*n);
	int *r = (int *)malloc(sizeof(int));
	*r = 2*(*n);
	free(p);
	hpxc_thread_exit(r);
    return r;
}

void my_launch()
{
    hpxc_thread_t thread;
	int *n = (int *)malloc(sizeof(int));
	*n = 120;
    hpxc_thread_create(&thread, NULL, hello_thread, n); 
	int *r;
	hpxc_thread_join(thread,(void **)&r);
	printf("r=%d\n",*r);
	free(r);
}

extern void hpxc_launch(int argc,char *argv[],void (*launch)());

#undef main
int main(int argc, char* argv[]) {
	hpxc_launch(argc,argv,my_launch);
	return 0;
}
