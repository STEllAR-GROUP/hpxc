//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpxc/threads.h>
#include <boost/atomic.hpp>

#include <errno.h>

struct thread_handle {
	boost::atomic<int> refc;
	hpx::threads::thread_id_type id;
	hpx::lcos::local::promise<void*> promise;
	hpx::lcos::future<void*> future;

	thread_handle() : refc(2) {}
};

void wrapper_function(
	thread_handle *handle,
	void *(*thread_function)(void*),
	void *arguments)
{
	try {
		handle->promise.set_value(thread_function(arguments));
	} catch(void *ret) {
		handle->promise.set_value(ret);
	}
	int r = --handle->refc;//__sync_fetch_and_add(&handle->refc,-1);
	if(r == 0) delete handle;
}

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
    	//boost::shared_ptr<hpx::lcos::local::promise<void*> > p =
        	//boost::make_shared<hpx::lcos::local::promise<void*> >();

		thread_handle *th = new thread_handle;
		th->future = th->promise.get_future();

        hpx::threads::thread_id_type id = 
            hpx::applier::register_thread(
                //HPX_STD_BIND(thread_function, arguments), 
                HPX_STD_BIND(wrapper_function, th, thread_function, arguments), 
                "hpxc_thread_create");
		th->id = id;
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

struct cond_internal {
   	std::vector<
		boost::shared_ptr<
			hpx::lcos::local::promise<int>
		>
	> waiting;
	cond_internal() : waiting() {}
};

extern "C"
{
	int hpxc_cond_init(hpxc_cond_t *cond,void *unused)
	{
		cond->handle = new cond_internal();
		return 0;
	}
	int hpxc_cond_wait(hpxc_cond_t *cond,hpxc_mutex_t *mutex)
	{
    	cond_internal *cond_in =
			reinterpret_cast<cond_internal *>(cond->handle);
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);

    	boost::shared_ptr<hpx::lcos::local::promise<int> > p =
        	boost::make_shared<hpx::lcos::local::promise<int> >();
		cond_in->waiting.push_back(p);

		lock->unlock();
		p->get_future().get();
		lock->lock();
		return 0;
	}
	int hpxc_cond_broadcast(hpxc_cond_t *cond)
	{
    	cond_internal *cond_in =
			reinterpret_cast<cond_internal *>(cond->handle);
		for(auto i=cond_in->waiting.begin(); i != cond_in->waiting.end();++i)
		{
			(*i)->set_value(0);
		}
		cond_in->waiting.clear();
		return 0;
	}
	int hpxc_cond_signal(hpxc_cond_t *cond)
	{
    	cond_internal *cond_in =
			reinterpret_cast<cond_internal *>(cond->handle);
		if(cond_in->waiting.size() > 0)
		{
			cond_in->waiting.erase(
				cond_in->waiting.begin(),
				cond_in->waiting.begin()+1);
		}
		return 0;
	}
	int hpxc_mutex_init(hpxc_mutex_t *mutex,void *ignored)
	{
		mutex->handle = new hpx::lcos::local::spinlock();
		return 0;
	}
	hpxc_mutex_t hpxc_mutex_alloc()
	{
		hpxc_mutex_t mut;
		mut.handle = new hpx::lcos::local::spinlock();
		return mut;
	}
	void hpxc_mutex_destroy(hpxc_mutex_t *mutex)
	{
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
		delete lock;
		mutex->handle = 0;
	}
	int hpxc_mutex_lock(hpxc_mutex_t *mutex)
	{
		if(mutex->handle == 0)
			return -1;
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
		lock->lock();
		return 0;
	}
	int hpxc_mutex_unlock(hpxc_mutex_t *mutex)
	{
		if(mutex->handle == 0)
			return -1;
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
		lock->unlock();
		return 0;
	}
	int hpxc_mutex_trylock(hpxc_mutex_t *mutex)
	{
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
		return lock->try_lock();
	}

    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENT: value_ptr.
    int hpxc_thread_join(
        hpxc_thread_t thread_id,
        void** value_ptr)
    {
		if(thread_id.handle == NULL)
			return -1;

        thread_handle *handle
            = reinterpret_cast<thread_handle*>(thread_id.handle);
		if(value_ptr != 0)
			*value_ptr = handle->future.get();
		else
			handle->future.get();
		int r = --handle->refc;//__sync_fetch_and_add(&handle->refc,-1);
		if(r == 0) {
			delete handle;
			thread_id.handle = NULL;
		}
		return 0;
    }

    int hpxc_thread_detach(
        hpxc_thread_t thread_id)
    {
		if(thread_id.handle == 0)
			return -1;

        thread_handle *handle =
            reinterpret_cast<thread_handle*>(thread_id.handle);
		//handle->future.cancel();
		int r = --handle->refc;//__sync_fetch_and_add(&handle->refc,-1);
		if(r == 0) {
			delete handle;
			thread_id.handle = NULL;
		}
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
