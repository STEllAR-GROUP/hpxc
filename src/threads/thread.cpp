//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpxc/threads.h>
#include <boost/atomic.hpp>

#include <errno.h>
#undef NDEBUG

const int MAGIC = 0xCAFEBABE;

struct thread_handle
{
	hpx::threads::thread_id_type id;
    int magic;
	boost::atomic<int> refc;
	hpx::lcos::local::promise<void*> promise;
	hpx::lcos::future<void*> future;
    int cancel_flags;

	thread_handle() : id(), magic(MAGIC), refc(2),
        promise(), future(promise.get_future()), cancel_flags(HPXC_THREAD_CANCEL_ENABLE) {}
};

thread_handle *get_thread_data(hpx::threads::thread_id_type id)
{
    thread_handle *thandle =
        reinterpret_cast<thread_handle*>(
            hpx::threads::get_thread_data(id));
    assert(thandle);
    assert(thandle->magic == MAGIC);
    return thandle;
}

thread_handle *get_thread_data(hpxc_thread_t thread)
{
    /*
    if(thread.handle == NULL)
        return NULL;
    hpx::threads::thread_id_type id =
        reinterpret_cast<hpx::threads::thread_id_type>(
            thread.handle);
    return get_thread_data(id);
    */
    thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
    return thandle;
}

void free_data(hpxc_thread_t thread)
{
    if(thread.handle == NULL)
        return;
    thread_handle *thandle =
        reinterpret_cast<thread_handle*>(thread.handle);
    assert(thandle->magic == MAGIC);
    hpx::threads::set_thread_data(thandle->id,(size_t)0);
    delete thandle;
    thread.handle = NULL;
}

void wrapper_function(
	thread_handle *thandle,
	void *(*thread_function)(void*),
	void *arguments)
{
    assert(thandle);
    assert(thandle->magic == MAGIC);
    hpx::threads::thread_self* self = hpx::threads::get_self_ptr();
    thandle->id = self->get_thread_id();
    self->set_thread_data(
        reinterpret_cast<size_t>(thandle));
    //assert(get_thread_data(thandle->id) == thandle);
	try {
		thandle->promise.set_value(thread_function(arguments));
	} catch(void *ret) {
		thandle->promise.set_value(ret);
	}
	int r = --thandle->refc;
	if(r == 0) { 
        delete thandle;
    }
}

struct hpxc_thread_attr_handle
{
	bool detach;
	bool remote;
	hpx::naming::id_type gid;
	hpxc_thread_attr_handle() : detach(false), remote(false), gid() {}
	
	void run_remote() {
		remote = true;
		hpx::find_here();
	}
	virtual ~hpxc_thread_attr_handle() {}
};

extern "C"
{
    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENT: attributes. 
	int hpxc_thread_attr_init(hpxc_thread_attr_t *attr)
	{
		hpxc_thread_attr_handle *handle= new hpxc_thread_attr_handle();
		if(handle == NULL)
			return ENOMEM;
		attr->handle = handle;
		return 0;
	}
	int hpxc_thread_attr_destroy(hpxc_thread_attr_t *attr)
	{
		if(attr->handle==NULL)
			return EINVAL;
		hpxc_thread_attr_handle *handle =
			reinterpret_cast<hpxc_thread_attr_handle *>(attr->handle);
		delete handle;
		attr->handle = NULL;
		return 0;
	}
	int hpxc_thread_attr_setdetachstate(hpxc_thread_attr_t *attr,int detach)
	{
		if(attr->handle==NULL)
			return EINVAL;
		hpxc_thread_attr_handle *handle =
			reinterpret_cast<hpxc_thread_attr_handle *>(attr->handle);
		handle->detach = detach ? true : false;
		return 0;
	}
	int hpxc_thread_attr_getdetachstate(hpxc_thread_attr_t *attr,int *detach)
	{
		if(attr->handle==NULL)
			return EINVAL;
		hpxc_thread_attr_handle *handle =
			reinterpret_cast<hpxc_thread_attr_handle *>(attr->handle);
		if(handle->detach)
			*detach = 1;
		else
			*detach = 0;
		return 0;
	}

    int hpxc_thread_create(
        hpxc_thread_t* thread, 
        hpxc_thread_attr_t const* attr,
        void* (*thread_function)(void*), 
        void* arguments)
    {
		thread_handle *thandle = new thread_handle;

		if(attr != NULL) {
            assert(0);
			hpxc_thread_attr_handle *handle =
				reinterpret_cast<hpxc_thread_attr_handle *>(attr->handle);
			if(handle->detach) {
                thandle->refc--;
				hpx::applier::register_thread(
						HPX_STD_BIND(thread_function, arguments), 
						"hpxc_thread_create");
				return 0;
			}
		}

        hpx::threads::thread_id_type id = 
            hpx::applier::register_thread(
                HPX_STD_BIND(wrapper_function, thandle, thread_function, arguments), 
                "hpxc_thread_create");
        thandle->id = id;
        thread->handle = reinterpret_cast<void*>(thandle);

        return 0;
    }
}

// IMPLEMENT: value_ptr.
inline void resume_thread(hpx::threads::thread_id_type id, void** value_ptr)
{
    hpx::threads::set_thread_state(id, hpx::threads::pending);
}

struct cond_handle
{
   	std::vector<
		boost::shared_ptr<
			hpx::lcos::local::promise<int>
		>
	> waiting;
	cond_handle() : waiting() {}
};

extern "C"
{
	int hpxc_cond_init(hpxc_cond_t *cond,void *unused)
	{
		cond->handle = new cond_handle();
		return 0;
	}
	int hpxc_cond_wait(hpxc_cond_t *cond,hpxc_mutex_t *mutex)
	{
    	cond_handle *cond_in =
			reinterpret_cast<cond_handle *>(cond->handle);
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
    	cond_handle *cond_in =
			reinterpret_cast<cond_handle *>(cond->handle);
		for(auto i=cond_in->waiting.begin(); i != cond_in->waiting.end();++i)
		{
			(*i)->set_value(0);
		}
		cond_in->waiting.clear();
		return 0;
	}
	int hpxc_cond_signal(hpxc_cond_t *cond)
	{
    	cond_handle *cond_in =
			reinterpret_cast<cond_handle *>(cond->handle);
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
		return { new hpx::lcos::local::spinlock() };
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
			return EINVAL;
		hpx::lcos::local::spinlock *lock =
			reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
		lock->lock();
		return 0;
	}
	int hpxc_mutex_unlock(hpxc_mutex_t *mutex)
	{
		if(mutex->handle == 0)
			return EINVAL;
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

    int hpxc_thread_testcancel()
    {
        hpxc_thread_t thread = hpxc_thread_self();

        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle==NULL)
            return EINVAL;

        int state = HPXC_THREAD_CANCEL_ENABLE|HPXC_THREAD_CANCELED;
        if((thandle->cancel_flags & state) == state) {
            throw (void *)NULL;
        }
        printf("testcancel=%x\n",thandle->cancel_flags);
        return 0;
    }

    int hpxc_thread_cancel(hpxc_thread_t thread)
    {
        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle == NULL)
            return ESRCH;
        if((thandle->cancel_flags & HPXC_THREAD_CANCEL_ENABLE) == HPXC_THREAD_CANCEL_ENABLE) {
            thandle->cancel_flags |= HPXC_THREAD_CANCELED;
            printf("cancel=%x\n",thandle->cancel_flags);
            return 0;
        } else {
            return EINVAL;
        }
    }
    int hpxc_thread_setcancelstate(int state,int *old_state)
    {
        hpxc_thread_t thread = hpxc_thread_self();

        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle==NULL)
            return EINVAL;

        *old_state = (thandle->cancel_flags & HPXC_THREAD_CANCELED);
        if(state) {
            thandle->cancel_flags |=  HPXC_THREAD_CANCELED;
        } else {
            thandle->cancel_flags &= ~HPXC_THREAD_CANCELED;
        }
        return 0;
    }
    int hpxc_thread_setcanceltype(int state,int *old_state)
    {
        *old_state = HPXC_THREAD_CANCEL_DEFERRED;
        if(state != HPXC_THREAD_CANCEL_DEFERRED)
            return EINVAL;
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // IMPLEMENT: value_ptr.
    int hpxc_thread_join(
        hpxc_thread_t thread,
        void** value_ptr)
    {
        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        assert(thandle->magic == MAGIC);
        if(thandle==NULL)
            return ESRCH;
		if(value_ptr != NULL)
			*value_ptr = thandle->future.get();
		else
			thandle->future.get();
		int r = --thandle->refc;
		if(r == 0) {
            free_data(thread);
		}
		return 0;
    }

    int hpxc_thread_detach(
        hpxc_thread_t thread)
    {
        thread_handle *thandle = 
            reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle==NULL)
            return ESRCH;
		int r = --thandle->refc;
		if(r == 0) {
            free_data(thread);
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
            hpxc_thread_t t = {
                reinterpret_cast<void*>(self->get_thread_data())
            };
            return t;
        }

        hpxc_thread_t t = { NULL };
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
