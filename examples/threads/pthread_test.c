//  Copyright (c) 2012-2013 Alexander Duchene
//  Copyright (c) 2022 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpxc/pthread_compatibility.h>
#include <stdio.h>

int retval = 0;

void* print_hello(void* nothing)
{
    printf("hello!\n");
    retval = 42;
    pthread_exit(&retval);
    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t test;
    pthread_create(&test, NULL, print_hello, NULL);
    int* result = NULL;
    pthread_join(test, (void**) &result);
    printf("Returned result: %d\n", *result);
    return 0;
}
