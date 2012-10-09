//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpxc/threads.h>

#include <errno.h>

void wrapper_function(
		boost::shared_ptr<hpx::lcos::local::promise<void*> > promise,
		void*(*thread_function)(void*),void *arguments)
{
	boost::shared_ptr<hpx::lcos::local::promise<void*> > dup = promise;
	try {
		promise->set_value(thread_function(arguments));
	} catch(void *ret) {
		promise->set_value(ret);
	}
}

struct thread_handle {
	hpx::threads::thread_id_type id;
	boost::shared_ptr<hpx::lcos::local::promise<void*> > promise;
	hpx::lcos::future<void*> future;
};

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
    	boost::shared_ptr<hpx::lcos::local::promise<void*> > p =
        	boost::make_shared<hpx::lcos::local::promise<void*> >();

        hpx::threads::thread_id_type id = 
            hpx::applier::register_thread(
                //HPX_STD_BIND(thread_function, arguments), 
                HPX_STD_BIND(wrapper_function, p, thread_function, arguments), 
                "hpxc_thread_create");

		std::cout << "thread_id = " << id << std::endl;

		thread_handle *th = new thread_handle;
		th->id = id;
		th->promise = p;
		th->future = p->get_future();
        hpxc_thread_t t = { th };
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

        thread_handle *th
            = reinterpret_cast<thread_handle*>(thread_id->handle);
		if(value_ptr != 0)
			*value_ptr = th->future.get();
		else
			th->future.get();
		delete th;
		return 0;
    }

    int hpxc_thread_detach(
        hpxc_thread_t* thread_id)
    {
        if (!thread_id)
            return ESRCH;

		if(thread_id->handle == 0)
			return 
        thread_handle *th
            = reinterpret_cast<thread_handle*>(thread_id->handle);
		th->future.cancel();
		delete th;
		return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // FIXME: What should I do if not called from an hpx-thread?
    // IMPLEMENT: value_ptr.
    void hpxc_thread_exit(void* value_ptr)
    {
		throw value_ptr;
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
	void show_thread()
	{
        hpx::threads::thread_self* self = hpx::threads::get_self_ptr();

        if (self)
        {
            std::cout << "self=" << self->get_thread_id() << std::endl;
        }
	}
}
