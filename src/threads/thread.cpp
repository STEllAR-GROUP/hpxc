//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpxc/threads.h>
#include <boost/atomic.hpp>

#include <errno.h>
#include <map>
#include <list>

const int MAGIC = 0xCAFEBABE;

//Possibly should encapsulate
struct tls_key{
    void (*destructor_function)(void*);

    tls_key(void (*destructor)(void*)):destructor_function(destructor){}
};

struct hpxc_return { void *handle; };

struct thread_handle
{
    hpx::threads::thread_id_type id;
#if defined(HPX_DEBUG)
    int magic;
#endif
    boost::atomic<int> refc;
    hpx::lcos::local::promise<void*> promise;
    hpx::lcos::future<void*> future;
    int cancel_flags;
    std::map<tls_key*,const void*> thread_local_storage;
    std::vector<hpx::util::function_nonser<void()> > cleanup_functions;

    thread_handle() : id(),
#if defined(HPX_DEBUG)
    magic(MAGIC),
#endif
        refc(2),
        promise(), future(promise.get_future()), cancel_flags(HPXC_THREAD_CANCEL_ENABLE) {}

    ~thread_handle();
};

thread_handle::~thread_handle(){
    //Clean up tls
    for(std::map<tls_key*,const void*>::iterator tls_iter=thread_local_storage.begin();
            tls_iter!=thread_local_storage.end();
            ++tls_iter){
        if((*tls_iter).first->destructor_function){
            ((*tls_iter).first->destructor_function)(const_cast<void*>((*tls_iter).second));
        }
    }
    //execute cleanup functions
    for(std::vector<hpx::util::function_nonser<void()> >::reverse_iterator d_iter= \
            cleanup_functions.rbegin();
            d_iter != cleanup_functions.rend();
            d_iter++){
        (*d_iter)();
    }
}

thread_handle *get_thread_data(hpx::threads::thread_id_type id)
{
    thread_handle *thandle =
        reinterpret_cast<thread_handle*>(
            hpx::threads::get_thread_data(id));
    BOOST_ASSERT(thandle);
    BOOST_ASSERT(thandle->magic == MAGIC);
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
    BOOST_ASSERT(thandle->magic == MAGIC);
    delete thandle;
    thread.handle = NULL;
}

void wrapper_function(
    thread_handle *thandle,
    void *(*thread_function)(void*),
    void *arguments)
{
    BOOST_ASSERT(thandle);
    BOOST_ASSERT(thandle->magic == MAGIC);
    hpx::threads::thread_self* self = hpx::threads::get_self_ptr();
    hpx::threads::set_thread_interruption_enabled(
        hpx::threads::get_self_id(),true);
    thandle->id = hpx::threads::get_self_id();
    self->set_thread_data(
        reinterpret_cast<size_t>(thandle));
    //BOOST_ASSERT(get_thread_data(thandle->id) == thandle);
    try {
        thandle->promise.set_value(thread_function(arguments));
    } catch(hpxc_return *ret) {
        thandle->promise.set_value(reinterpret_cast<void*>(ret));
    // Handle cancelation
    } catch(hpx::thread_interrupted e) {
        // release all
        thandle->promise.set_value(HPXC_CANCELED);
    } catch(hpx::exception e) {
            throw;
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
    hpx::threads::thread_stacksize stacksize;
    hpxc_thread_attr_handle() : detach(false), remote(false), gid(),
        stacksize(hpx::threads::thread_stacksize_default) {}

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

    int hpxc_thread_attr_setscope(hpxc_thread_attr_t *attr, int scope){
        return 0;
    }

    int hpxc_thread_attr_getscope(hpxc_thread_attr_t *attr, int* scope){
        *scope=HPXC_THREAD_SCOPE_SYSTEM;
        return 0;
    }

    int hpxc_thread_attr_setstacksize(hpxc_thread_attr_t *attr, size_t stacksize){
        if(attr->handle==NULL){
            return EINVAL;
        }
        hpxc_thread_attr_handle *handle=
            reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
        if(stacksize > HPX_MEDIUM_STACK_SIZE){
            if(stacksize > HPX_LARGE_STACK_SIZE){
                handle->stacksize=hpx::threads::thread_stacksize_huge;
            }
            else{
                handle->stacksize=hpx::threads::thread_stacksize_large;
            }

        }
        else{
            if(stacksize > HPX_SMALL_STACK_SIZE){
                handle->stacksize=hpx::threads::thread_stacksize_medium;
            }
            else{
                handle->stacksize=hpx::threads::thread_stacksize_small;
            }
        }

        return 0;
    }

    int hpxc_thread_attr_getstacksize(const hpxc_thread_attr_t* attr, size_t* stacksize){
        if(attr->handle==NULL){
            return EINVAL;
        }
        hpxc_thread_attr_handle *handle=
            reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
        switch (handle->stacksize){
            case hpx::threads::thread_stacksize_small:
                *stacksize=HPX_SMALL_STACK_SIZE;
                break;
            case hpx::threads::thread_stacksize_medium:
                *stacksize=HPX_MEDIUM_STACK_SIZE;
                break;
            case hpx::threads::thread_stacksize_large:
                *stacksize=HPX_LARGE_STACK_SIZE;
                break;
            case hpx::threads::thread_stacksize_huge:
                *stacksize=HPX_HUGE_STACK_SIZE;
                break;
            case hpx::threads::thread_stacksize_nostack:
                HPX_ASSERT(false); // nostack not supported in hpxc
                *stacksize=0;
                break;
        }
        return 0;
    }

    int hpxc_thread_setaffinity_np(hpxc_thread_t thread, size_t cpusetsize, const hpxc_cpu_set_t *cpuset){
        return 0;
    }

    int hpxc_thread_getaffinity_np(hpxc_thread_t thread, size_t cpusetsize, hpxc_cpu_set_t *cpuset){
        //what should cpuset be set to?
        cpuset->handle=NULL;
        return 0;
    }

    int hpxc_thread_create(
        hpxc_thread_t* thread,
        hpxc_thread_attr_t const* attr,
        void* (*thread_function)(void*),
        void* arguments)
    {
        thread_handle *thandle = new thread_handle;
        hpx::threads::thread_stacksize stacksize=hpx::threads::thread_stacksize_default;
        if(attr != NULL) {
            hpxc_thread_attr_handle *handle =
                reinterpret_cast<hpxc_thread_attr_handle *>(attr->handle);
            stacksize=handle->stacksize;
            if(handle->detach) {
                thandle->refc--;
                hpx::applier::register_thread(
                        hpx::util::bind(thread_function, arguments),
                        "hpxc_thread_create",
                        hpx::threads::pending,
                        true,
                        hpx::threads::thread_priority_normal,
                        -1,
                        stacksize);
                return 0;
            }
        }

        hpx::threads::thread_id_type id =
            hpx::applier::register_thread(
                hpx::util::bind(wrapper_function, thandle, thread_function, arguments),
                "hpxc_thread_create",
                hpx::threads::pending,
                true,
                hpx::threads::thread_priority_normal,
                -1,
                stacksize);
        thandle->id = id;
        thread->handle = reinterpret_cast<void*>(thandle);
        hpx::threads::set_thread_interruption_enabled(
            thandle->id,true);

        return 0;
    }
}

// IMPLEMENT: value_ptr.
inline void resume_thread(hpx::threads::thread_id_type id, void** value_ptr)
{
    hpx::threads::set_thread_state(id, hpx::threads::pending);
}

#if defined(HPX_HAVE_VERIFY_LOCKS)
namespace hpx { namespace util
{
    template <>
    struct ignore_while_checking<hpx::lcos::local::spinlock>
    {
        ignore_while_checking(hpx::lcos::local::spinlock* lock)
          : mtx_(lock)
        {
            ignore_lock(mtx_);
        }

        ~ignore_while_checking()
        {
            reset_ignored(mtx_);
        }

        void const* mtx_;
    };
}}
#endif

extern "C"
{
    int hpxc_cond_init(hpxc_cond_t *cond,void *unused)
    {
        cond->handle = new hpx::lcos::local::condition_variable();
        return 0;
    }
    int hpxc_cond_wait(hpxc_cond_t *cond, hpxc_mutex_t *mutex)
    {
        hpx::lcos::local::condition_variable *cond_var =
            reinterpret_cast<hpx::lcos::local::condition_variable *>(cond->handle);
        hpx::lcos::local::spinlock *lock =
            reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);

        try {
            cond_var->wait(*lock);
        }
        catch (hpx::exception&) {
            return EINVAL;
        }

        return 0;
    }

    int hpxc_cond_timedwait(hpxc_cond_t *cond,hpxc_mutex_t *mutex, const struct timespec *tm)
    {
/*
        hpx::lcos::local::condition_variable *cond_var =
            reinterpret_cast<hpx::lcos::local::condition_variable *>(cond->handle);
        hpx::lcos::local::spinlock *lock =
            reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
*/
        BOOST_ASSERT(false); // HAVING TROUBLE TO CONVERT timespec to boost::chrono
/*
        hpx::lcos::local::cv_status ret;

        try{
            ret = cond_var->wait_until(*lock, tn);
        } catch(hpx::exception& e) {
            return EINVAL;
        }

        switch(ret)
        {
            case timeout:
                return ETIMEDOUT;
            case error:
                return EINVAL;
            default:
                break;
        }
  */
        return 0;
    }
    int hpxc_cond_broadcast(hpxc_cond_t *cond)
    {
        hpx::lcos::local::condition_variable *cond_var =
            reinterpret_cast<hpx::lcos::local::condition_variable *>(cond->handle);

        try{
            cond_var->notify_all();
        } catch(hpx::exception e) {
            return EINVAL;
        }

        return 0;
    }
    int hpxc_cond_signal(hpxc_cond_t *cond)
    {
        hpx::lcos::local::condition_variable *cond_var =
            reinterpret_cast<hpx::lcos::local::condition_variable *>(cond->handle);

        try{
            cond_var->notify_one();
        } catch(hpx::exception e) {
            return EINVAL;
        }

        return 0;
    }
    int hpxc_cond_destroy(hpxc_cond_t *cond)
    {
        hpx::lcos::local::condition_variable *cond_var =
            reinterpret_cast<hpx::lcos::local::condition_variable *>(cond->handle);
        delete cond_var;
        cond->handle = 0;
        return 0;
    }
    int hpxc_mutex_init(hpxc_mutex_t *mutex,void *ignored)
    {
        mutex->handle = new hpx::lcos::local::spinlock();
        return 0;
    }
    hpxc_mutex_t hpxc_mutex_alloc()
    {
        hpxc_mutex_t mtx = { new hpx::lcos::local::spinlock() };
        return mtx;
    }
    int hpxc_mutex_destroy(hpxc_mutex_t *mutex)
    {
        hpx::lcos::local::spinlock *lock =
            reinterpret_cast<hpx::lcos::local::spinlock*>(mutex->handle);
        delete lock;
        mutex->handle = 0;
        return 0;
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

    int hpxc_spin_init(hpxc_spinlock_t *mut, void* unused){
        return hpxc_mutex_init(mut, unused);
    }

    int hpxc_spin_lock(hpxc_spinlock_t *mut){
        return hpxc_mutex_lock(mut);
    }

    int hpxc_spin_unlock(hpxc_spinlock_t *mut){
        return hpxc_mutex_unlock(mut);
    }

    int hpxc_spin_trylock(hpxc_spinlock_t *mut){
        return hpxc_mutex_trylock(mut);
    }

    int hpxc_spin_destroy(hpxc_spinlock_t *mut){
        return hpxc_mutex_destroy(mut);
    }

    int hpxc_thread_testcancel()
    {
        hpxc_thread_t thread = hpxc_thread_self();

        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle==NULL)
            return EINVAL;
        BOOST_ASSERT(thandle->magic == MAGIC);

        hpx::this_thread::interruption_point();
        return EINVAL;
    }

    int hpxc_thread_cancel(hpxc_thread_t thread)
    {
        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle == NULL)
            return ESRCH;
        BOOST_ASSERT(thandle->magic == MAGIC);

        hpx::error_code ec;
        hpx::threads::interrupt_thread(thandle->id,ec);
        if(ec) {
            return EINVAL;
        } else {
            return 0;
        }
    }
    int hpxc_thread_setcancelstate(int state,int *old_state)
    {
        hpxc_thread_t thread = hpxc_thread_self();

        thread_handle *thandle = reinterpret_cast<thread_handle*>(thread.handle);
        if(thandle==NULL)
            return EINVAL;
        BOOST_ASSERT(thandle->magic == MAGIC);

        *old_state = (thandle->cancel_flags & HPXC_THREAD_CANCELED);
        if(state == HPXC_THREAD_CANCELED) {
            thandle->cancel_flags |=  HPXC_THREAD_CANCELED;
            hpx::threads::interrupt_thread(
                thandle->id);
        } else {
            thandle->cancel_flags &= ~HPXC_THREAD_CANCELED;
            // TODO: not yet supported by HPX
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
        if(thandle==NULL)
            return ESRCH;
        BOOST_ASSERT(thandle->magic == MAGIC);
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
        BOOST_ASSERT(thandle->magic == MAGIC);
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
        // FIXME: is it safe to throw from an extern "C" function?
        // FIXME: Let's not throw a void* but package it up into a proper
        //        exception object (derived from hpx::exception)
        throw reinterpret_cast<hpxc_return*>(value_ptr);
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

    int hpxc_key_create(hpxc_key_t *key, void (*destructor)(void*)){
        key->handle=new tls_key(destructor);
        return 0;
    }

    int hpxc_key_delete(hpxc_key_t key){
        delete ((tls_key*)(key.handle));
        return 0;
    }

    int hpxc_setspecific(hpxc_key_t key, const void* value){
        thread_handle* self=::get_thread_data(hpx::threads::get_self_id());
        self->thread_local_storage[(tls_key*)key.handle]=value;
        return 0;
    }

    void* hpxc_getspecific(hpxc_key_t key){
        thread_handle* self=::get_thread_data(hpx::threads::get_self_id());
        return const_cast<void*>(self->thread_local_storage[(tls_key*)key.handle]);
        return 0;
    }

    int hpxc_thread_equal(hpxc_thread_t t1, hpxc_thread_t t2){
        return t1.handle==t2.handle;
    }

    void hpxc_thread_cleanup_push(void (*routine)(void*), void* arg){
        thread_handle* self=::get_thread_data(hpx::threads::get_self_id());
        self->cleanup_functions.push_back(hpx::util::bind(routine, arg));
        return;
    }

    void hpxc_thread_cleanup_pop(int execute){
        thread_handle* self=::get_thread_data(hpx::threads::get_self_id());
        if(execute){
            self->cleanup_functions.back()();
        }
        self->cleanup_functions.pop_back();
    }

}
