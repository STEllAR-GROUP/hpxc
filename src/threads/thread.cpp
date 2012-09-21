//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpxc/threads.h>

#include <errno.h>

extern "C"
{
    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENT: attributes. 
    int hpxc_thread_create(
        hpxc_thread_t* thread_id, 
        hpxc_thread_attr_t const* attributes,
        void* (*thread_function)(void*), 
        void* arguments)
    {
        hpx::threads::thread_id_type id = 
            hpx::applier::register_thread(
                HPX_STD_BIND(thread_function, arguments), 
                "hpxc_thread_create");

        hpxc_thread_t t = { id };
        *thread_id = t;

        return 0;
    }
}

// IMPLEMENT: value_ptr.
inline void resume_thread(hpx::threads::thread_id_type id, void** value_ptr)
{
    hpx::threads::set_thread_state(id, hpx::threads::pending);
}

extern "C"
{
    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENT: value_ptr.
    int hpxc_thread_join(
        hpxc_thread_t* thread_id,
        void** value_ptr)
    {
        if (!thread_id)
            return ESRCH;

        hpx::threads::thread_id_type target_id
            = reinterpret_cast<hpx::threads::thread_id_type>(thread_id->handle);

        if (hpx::threads::invalid_thread_id == target_id)
            return ESRCH;

        // register callback function to be called when thread exits
        hpx::threads::thread_id_type this_id
            = hpx::threads::get_self().get_thread_id();

         // make sure the calling thread isn't the target thread
        if (this_id == target_id)
            return EDEADLK;

        hpx::this_thread::interruption_point();

        if (hpx::threads::add_thread_exit_callback(target_id,
                HPX_STD_BIND(&resume_thread, this_id, value_ptr)))
        {
            // wait for thread to be terminated
            hpx::this_thread::suspend(hpx::threads::suspended,
                "hpxc_thread_join");
            return 0;
        }
        else
        {
            return EINVAL;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // FIXME: What should I do if not called from an hpx-thread?
    // IMPLEMENT: value_ptr.
    void hpxc_thread_exit(void* value_ptr)
    {
        hpx::threads::thread_self* self = hpx::threads::get_self_ptr();
        if (self)
            self->yield(hpx::threads::terminated);    // this will not return
    }

    ///////////////////////////////////////////////////////////////////////////
    hpxc_thread_t hpxc_thread_self()
    {
        hpx::threads::thread_self* self = hpx::threads::get_self_ptr();

        if (self)
        {
            hpxc_thread_t t = { self->get_thread_id() };
            return t;
        }

        hpxc_thread_t t = { hpx::threads::invalid_thread_id };
        return t;         
    }
}

