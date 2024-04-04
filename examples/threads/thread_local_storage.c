//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <assert.h>
#include <hpxc/threads.h>
#include <hpxc/util/wrap_main.h>
#include <stdio.h>

hpxc_key_t key;
hpxc_mutex_t counter_lock;
int counter;

hpxc_mutex_t io_lock;

void destructor_f(void* tls_data)
{
    printf("Destructor called on %p\n", tls_data);
}

void* creates_key(void* inputs)
{
    hpxc_key_create(&key, &destructor_f);
    return NULL;
}

void* uses_key(void* inputs)
{
    size_t num = 0;
    double some_work = 1;
    int spin;
    size_t result;

    assert(hpxc_getspecific(key) == NULL);
    hpxc_mutex_lock(&counter_lock);
    num = counter++;
    hpxc_mutex_unlock(&counter_lock);
    hpxc_setspecific(key, (void*) num);

    for (spin = 0; spin < 1000000; spin++)
    {
        some_work = some_work * 1.1;
    }

    result = (size_t) hpxc_getspecific(key);
    assert(result == num);

    printf("Expected %zu, got %zu\n", num, result);
    return NULL;
}

#define NTHREADS 100

int main(int argc, char* argv[])
{
    hpxc_thread_t threads[NTHREADS];
    hpxc_thread_t init;
    int i;

    counter = 9;
    counter_lock = HPXC_MUTEX_INITIALIZER;
    io_lock = HPXC_MUTEX_INITIALIZER;

    hpxc_thread_create(&init, NULL, creates_key, NULL);
    hpxc_thread_join(init, NULL);

    for (i = 0; i < NTHREADS; i++)
    {
        hpxc_thread_create(&(threads[i]), NULL, uses_key, NULL);
    }
    for (i = 0; i < NTHREADS; i++)
    {
        hpxc_thread_join(threads[i], NULL);
    }

    return 0;
}
