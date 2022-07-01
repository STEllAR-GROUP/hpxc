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

int hpxc_thread_testcancel();
int hpxc_thread_cancel(hpxc_thread_t);

void* until_I_die(void* p)
{
    while (1)
    {
        hpxc_thread_testcancel();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    void* retval;

    hpxc_thread_t once;
    hpxc_thread_create(&once, NULL, until_I_die, NULL);
    hpxc_thread_cancel(once);
    hpxc_thread_join(once, &retval);

    if (retval != HPXC_CANCELED)
        printf("Something went wrong!\n");
    else
        printf("Cancelation was successful.\n");

    return 0;
}
