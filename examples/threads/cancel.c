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

void* until_I_die(void* p)
{
    while(1)
        hpxc_thread_testcancel();
    return 0;
}

void my_init()
{
    hpxc_thread_t once;
    hpxc_thread_create(&once,NULL,until_I_die,NULL);
    hpxc_thread_cancel(once);
    hpxc_thread_join(once,NULL);
    printf("Cancelation was successful.\n");
}

int main(int argc, char* argv[]) {
    hpxc_init(my_init,argc,argv);
    return 0;
}

