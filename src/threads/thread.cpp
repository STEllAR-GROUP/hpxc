//  Copyright (c) 2007-2022 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-lelbach
//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>

#include <hpxc/config.h>
#include <hpxc/threads.h>
#include <hpxc/threads/readers_writers_mutex.hpp>
#include <hpxc/util/init_hpx.hpp>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csetjmp>
#include <list>
#include <map>

const int MAGIC = 0xCAFEBABE;

// Possibly should encapsulate
struct tls_key
{
    void (*destructor_function)(void*) = nullptr;

    tls_key(void (*destructor)(void*))
      : destructor_function(destructor)
    {
    }
};

struct thread_handle
{
    hpx::threads::thread_id_ref_type id;
#if defined(HPX_DEBUG)
    int magic;
#endif
    std::atomic<int> refc;
    hpx::promise<void*> promise;
    hpx::future<void*> future;
    int cancel_flags;
    std::map<tls_key*, const void*> thread_local_storage;
    std::vector<hpx::function<void()>> cleanup_functions;
    void* retval;
    std::jmp_buf env;

    thread_handle()
      : id()
#if defined(HPX_DEBUG)
      , magic(MAGIC)
#endif
      , refc(2)
      , promise()
      , future(promise.get_future())
      , cancel_flags(HPXC_THREAD_CANCEL_ENABLE)
      , retval(nullptr)
    {
    }

    ~thread_handle();
};

thread_handle::~thread_handle()
{
    // Clean up tls
    for (auto tls_iter = thread_local_storage.begin();
         tls_iter != thread_local_storage.end(); ++tls_iter)
    {
        if ((*tls_iter).first->destructor_function)
        {
            ((*tls_iter).first->destructor_function)(
                const_cast<void*>((*tls_iter).second));
        }
    }

    // execute cleanup functions
    for (auto d_iter = cleanup_functions.rbegin();
         d_iter != cleanup_functions.rend(); d_iter++)
    {
        (*d_iter)();
    }
}

thread_handle* get_thread_data(hpx::threads::thread_id_type id)
{
    auto* thandle =
        reinterpret_cast<thread_handle*>(hpx::threads::get_thread_data(id));
    HPX_ASSERT(thandle);
    HPX_ASSERT(thandle->magic == MAGIC);
    return thandle;
}

thread_handle* get_thread_data(hpxc_thread_t thread)
{
    thread_handle* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    HPX_ASSERT(thandle->magic == MAGIC);
    return thandle;
}

void free_data(hpxc_thread_t thread)
{
    if (thread.handle == nullptr)
        return;
    thread_handle* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    HPX_ASSERT(thandle->magic == MAGIC);
    delete thandle;
    thread.handle = nullptr;
}

template <typename F>
struct do_on_exit
{
    do_on_exit(F&& f)
      : f_(f)
    {
    }

    ~do_on_exit()
    {
        f_();
    }

    F f_;
};

void wrapper_function(
    thread_handle* thandle, void* (*thread_function)(void*), void* arguments)
{
    HPX_ASSERT(thandle);
    HPX_ASSERT(thandle->magic == MAGIC);

    do_on_exit _([&]() {
        int r = --thandle->refc;
        if (r == 0)
        {
            delete thandle;
        }
    });

    hpx::threads::thread_self* self = hpx::threads::get_self_ptr();
    hpx::threads::set_thread_interruption_enabled(
        hpx::threads::get_self_id(), true);

    thandle->id = hpx::threads::get_self_id();
    self->set_thread_data(reinterpret_cast<size_t>(thandle));

    // HPX_ASSERT(get_thread_data(thandle->id) == thandle);
    try
    {
        if (!setjmp(thandle->env))
        {
            thandle->retval = thread_function(arguments);
        }
        thandle->promise.set_value(thandle->retval);
    }
    catch (hpx::thread_interrupted const&)
    {
        thandle->promise.set_value(HPXC_CANCELED);
    }
    catch (...)
    {
        thandle->promise.set_value(HPXC_CANCELED);
        throw;
    }
}

struct hpxc_thread_attr_handle
{
    bool detach;
    hpx::threads::thread_stacksize stacksize;

    constexpr hpxc_thread_attr_handle() noexcept
      : detach(false)
      , stacksize(hpx::threads::thread_stacksize::default_)
    {
    }
};

extern "C" {

///////////////////////////////////////////////////////////////////////////
// IMPLEMENT: attributes.
int hpxc_thread_attr_init(hpxc_thread_attr_t* attr)
{
    if (attr == nullptr)
        return EINVAL;
    try
    {
        hpxc_thread_attr_handle* handle = new hpxc_thread_attr_handle();
        attr->handle = handle;
    }
    catch (...)
    {
        attr->handle = nullptr;
        return ENOMEM;
    }
    return 0;
}

int hpxc_thread_attr_destroy(hpxc_thread_attr_t* attr)
{
    if (attr == nullptr || attr->handle == nullptr)
        return EINVAL;
    auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
    attr->handle = nullptr;
    delete handle;
    return 0;
}

int hpxc_thread_attr_setdetachstate(hpxc_thread_attr_t* attr, int detach)
{
    if (attr == nullptr)
        return EINVAL;
    auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
    if (handle == nullptr)
        return EINVAL;
    handle->detach = detach ? true : false;
    return 0;
}

int hpxc_thread_attr_getdetachstate(hpxc_thread_attr_t* attr, int* detach)
{
    if (attr == nullptr)
        return EINVAL;
    auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
    if (handle == nullptr)
        return EINVAL;
    if (handle->detach)
        *detach = 1;
    else
        *detach = 0;
    return 0;
}

int hpxc_thread_attr_setscope(hpxc_thread_attr_t* attr, int scope)
{
    return 0;
}

int hpxc_thread_attr_getscope(hpxc_thread_attr_t* attr, int* scope)
{
    if (attr == nullptr)
        return EINVAL;
    *scope = HPXC_THREAD_SCOPE_SYSTEM;
    return 0;
}

int hpxc_thread_attr_setstacksize(hpxc_thread_attr_t* attr, size_t stacksize)
{
    if (attr == nullptr)
        return EINVAL;
    auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
    if (handle == nullptr)
        return EINVAL;
    if (stacksize > HPX_MEDIUM_STACK_SIZE)
    {
        if (stacksize > HPX_LARGE_STACK_SIZE)
        {
            handle->stacksize = hpx::threads::thread_stacksize::huge;
        }
        else
        {
            handle->stacksize = hpx::threads::thread_stacksize::large;
        }
    }
    else if (stacksize > HPX_SMALL_STACK_SIZE)
    {
        handle->stacksize = hpx::threads::thread_stacksize::medium;
    }
    else
    {
        handle->stacksize = hpx::threads::thread_stacksize::small_;
    }

    return 0;
}

int hpxc_thread_attr_getstacksize(
    const hpxc_thread_attr_t* attr, size_t* stacksize)
{
    if (attr == nullptr)
        return EINVAL;
    auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
    if (handle == nullptr)
        return EINVAL;
    switch (handle->stacksize)
    {
    case hpx::threads::thread_stacksize::small_:
        *stacksize = HPX_SMALL_STACK_SIZE;
        break;

    case hpx::threads::thread_stacksize::medium:
        *stacksize = HPX_MEDIUM_STACK_SIZE;
        break;

    case hpx::threads::thread_stacksize::large:
        *stacksize = HPX_LARGE_STACK_SIZE;
        break;

    case hpx::threads::thread_stacksize::huge:
        *stacksize = HPX_HUGE_STACK_SIZE;
        break;

    case hpx::threads::thread_stacksize::nostack:
        HPX_ASSERT(false);    // nostack not supported in hpxc
        *stacksize = 0;
        break;
    default:
        break;
    }
    return 0;
}

int hpxc_thread_setaffinity_np(
    hpxc_thread_t thread, size_t cpusetsize, const hpxc_cpu_set_t* cpuset)
{
    return 0;
}

int hpxc_thread_getaffinity_np(
    hpxc_thread_t thread, size_t cpusetsize, hpxc_cpu_set_t* cpuset)
{
    if (cpuset == nullptr)
        return EINVAL;
    // what should cpuset be set to?
    cpuset->handle = nullptr;
    return 0;
}

int hpxc_thread_create(hpxc_thread_t* thread, hpxc_thread_attr_t const* attr,
    void* (*thread_function)(void*), void* arguments)
{
    thread_handle* thandle = nullptr;
    try
    {
        thandle = new thread_handle;
    }
    catch (...)
    {
        return ENOMEM;
    }

    hpx::threads::thread_stacksize stacksize =
        hpx::threads::thread_stacksize::default_;
    if (attr != nullptr)
    {
        auto* handle = reinterpret_cast<hpxc_thread_attr_handle*>(attr->handle);
        if (handle == nullptr)
            return EINVAL;
        stacksize = handle->stacksize;
        if (handle->detach)
        {
            --thandle->refc;

            hpx::threads::thread_init_data data(
                hpx::threads::make_thread_function_nullary(
                    hpx::util::deferred_call(&wrapper_function, thandle,
                        thread_function, arguments)),
                "hpxc_thread_create", hpx::threads::thread_priority::normal,
                hpx::threads::thread_schedule_hint(), stacksize,
                hpx::threads::thread_schedule_state::pending, true);

            hpx::threads::register_thread(data);
            return 0;
        }
    }

    hpx::threads::thread_init_data data(
        hpx::threads::make_thread_function_nullary(hpx::util::deferred_call(
            &wrapper_function, thandle, thread_function, arguments)),
        "hpxc_thread_create", hpx::threads::thread_priority::normal,
        hpx::threads::thread_schedule_hint(), stacksize,
        hpx::threads::thread_schedule_state::pending, true);

    thandle->id = hpx::threads::register_thread(data);
    thread->handle = reinterpret_cast<void*>(thandle);
    hpx::threads::set_thread_interruption_enabled(thandle->id.noref(), true);

    return 0;
}

}    // extern "C"

// IMPLEMENT: value_ptr.
inline void resume_thread(hpx::threads::thread_id_type id, void** value_ptr)
{
    hpx::threads::set_thread_state(
        id, hpx::threads::thread_schedule_state::pending);
}

extern "C" {

int hpxc_cond_init(hpxc_cond_t* cond, void* unused)
{
    if (cond == nullptr)
        return EINVAL;
    try
    {
        cond->handle = new hpx::condition_variable();
    }
    catch (...)
    {
        cond->handle = nullptr;
        return ENOMEM;
    }
    return 0;
}

int hpxc_cond_wait(hpxc_cond_t* cond, hpxc_mutex_t* mutex)
{
    if (cond == nullptr || mutex == nullptr)
        return EINVAL;

    auto* cond_var = reinterpret_cast<hpx::condition_variable*>(cond->handle);
    if (cond_var == nullptr)
        return EINVAL;

    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    if (lock == nullptr)
        return EINVAL;

    try
    {
        std::unique_lock ul(*lock, std::adopt_lock_t());
        cond_var->wait(ul);
        ul.release();
    }
    catch (hpx::exception&)
    {
        return EINVAL;
    }

    return 0;
}

int hpxc_cond_timedwait(
    hpxc_cond_t* cond, hpxc_mutex_t* mutex, const struct timespec* tm)
{
    if (cond == nullptr || mutex == nullptr || tm == nullptr)
        return EINVAL;

    auto* cond_var = reinterpret_cast<hpx::condition_variable*>(cond->handle);
    if (cond_var == nullptr)
        return EINVAL;

    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    if (lock == nullptr)
        return EINVAL;

    hpx::cv_status ret;
    try
    {
        auto duration = std::chrono::seconds(tm->tv_sec) +
            std::chrono::nanoseconds(tm->tv_nsec);

        std::unique_lock ul(*lock, std::adopt_lock_t());
        ret = cond_var->wait_for(
            ul, std::chrono::duration_cast<std::chrono::nanoseconds>(duration));
        ul.release();
    }
    catch (...)
    {
        return EINVAL;
    }

    switch (ret)
    {
    case hpx::cv_status::timeout:
        return ETIMEDOUT;

    case hpx::cv_status::error:
        return EINVAL;

    default:
        break;
    }
    return 0;
}

int hpxc_cond_broadcast(hpxc_cond_t* cond)
{
    if (cond == nullptr)
        return EINVAL;

    auto* cond_var = reinterpret_cast<hpx::condition_variable*>(cond->handle);
    if (cond_var == nullptr)
        return EINVAL;

    try
    {
        cond_var->notify_all();
    }
    catch (...)
    {
        return EINVAL;
    }

    return 0;
}

int hpxc_cond_signal(hpxc_cond_t* cond)
{
    if (cond == nullptr)
        return EINVAL;

    auto* cond_var = reinterpret_cast<hpx::condition_variable*>(cond->handle);
    if (cond_var == nullptr)
        return EINVAL;

    try
    {
        cond_var->notify_one();
    }
    catch (...)
    {
        return EINVAL;
    }

    return 0;
}

int hpxc_cond_destroy(hpxc_cond_t* cond)
{
    if (cond == nullptr)
        return EINVAL;

    auto* cond_var = reinterpret_cast<hpx::condition_variable*>(cond->handle);
    cond->handle = nullptr;

    delete cond_var;
    return 0;
}

int hpxc_mutex_init(hpxc_mutex_t* mutex, void* ignored)
{
    if (mutex == nullptr)
        return EINVAL;

    try
    {
        mutex->handle = new hpx::spinlock();
    }
    catch (...)
    {
        mutex->handle = nullptr;
        return ENOMEM;
    }
    return 0;
}

hpxc_mutex_t hpxc_mutex_alloc()
{
    hpxc_mutex_t mtx;
    hpxc_mutex_init(&mtx, nullptr);
    return mtx;
}

int hpxc_mutex_destroy(hpxc_mutex_t* mutex)
{
    if (mutex == nullptr)
        return EINVAL;

    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    mutex->handle = nullptr;

    delete lock;
    return 0;
}

int hpxc_mutex_lock(hpxc_mutex_t* mutex)
{
    if (mutex == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    if (lock == nullptr)
        return EINVAL;
    lock->lock();
    return 0;
}

int hpxc_mutex_unlock(hpxc_mutex_t* mutex)
{
    if (mutex == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    if (lock == nullptr)
        return EINVAL;
    lock->unlock();
    return 0;
}

int hpxc_mutex_trylock(hpxc_mutex_t* mutex)
{
    if (mutex == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::spinlock*>(mutex->handle);
    return lock->try_lock();
}

int hpxc_spin_init(hpxc_spinlock_t* mut, void* unused)
{
    return hpxc_mutex_init(mut, unused);
}

int hpxc_spin_lock(hpxc_spinlock_t* mut)
{
    return hpxc_mutex_lock(mut);
}

int hpxc_spin_unlock(hpxc_spinlock_t* mut)
{
    return hpxc_mutex_unlock(mut);
}

int hpxc_spin_trylock(hpxc_spinlock_t* mut)
{
    return hpxc_mutex_trylock(mut);
}

int hpxc_spin_destroy(hpxc_spinlock_t* mut)
{
    return hpxc_mutex_destroy(mut);
}

int hpxc_thread_testcancel()
{
    hpxc_thread_t thread = hpxc_thread_self();

    thread_handle* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    if (thandle == nullptr)
        return EINVAL;
    HPX_ASSERT(thandle->magic == MAGIC);

    hpx::this_thread::interruption_point();
    return EINVAL;
}

int hpxc_thread_cancel(hpxc_thread_t thread)
{
    auto* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    if (thandle == nullptr)
        return ESRCH;
    HPX_ASSERT(thandle->magic == MAGIC);

    hpx::error_code ec(hpx::throwmode::lightweight);
    hpx::threads::interrupt_thread(thandle->id.noref(), ec);
    if (ec)
    {
        return EINVAL;
    }
    else
    {
        return 0;
    }
}

int hpxc_thread_setcancelstate(int state, int* old_state)
{
    hpxc_thread_t thread = hpxc_thread_self();

    auto* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    if (thandle == nullptr)
        return EINVAL;
    HPX_ASSERT(thandle->magic == MAGIC);

    *old_state = (thandle->cancel_flags & HPXC_THREAD_CANCELED);
    if (state == HPXC_THREAD_CANCELED)
    {
        thandle->cancel_flags |= HPXC_THREAD_CANCELED;
        hpx::threads::interrupt_thread(thandle->id.noref());
    }
    else
    {
        thandle->cancel_flags &= ~HPXC_THREAD_CANCELED;
        // TODO: not yet supported by HPX
    }
    return 0;
}

int hpxc_thread_setcanceltype(int state, int* old_state)
{
    if (old_state == nullptr)
        return EINVAL;
    *old_state = HPXC_THREAD_CANCEL_DEFERRED;
    if (state != HPXC_THREAD_CANCEL_DEFERRED)
        return EINVAL;
    return 0;
}

///////////////////////////////////////////////////////////////////////////
// IMPLEMENT: value_ptr.
int hpxc_thread_join(hpxc_thread_t thread, void** value_ptr)
{
    auto* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    if (thandle == nullptr)
        return ESRCH;
    HPX_ASSERT(thandle->magic == MAGIC);
    if (value_ptr != nullptr)
        *value_ptr = thandle->future.get();
    else
        thandle->future.get();
    int r = --thandle->refc;
    if (r == 0)
    {
        free_data(thread);
    }
    return 0;
}

int hpxc_thread_detach(hpxc_thread_t thread)
{
    auto* thandle = reinterpret_cast<thread_handle*>(thread.handle);
    if (thandle == nullptr)
        return ESRCH;
    HPX_ASSERT(thandle->magic == MAGIC);
    int r = --thandle->refc;
    if (r == 0)
    {
        free_data(thread);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////
// FIXME: What should I do if not called from an hpx-thread?
void hpxc_thread_exit(void* value_ptr)
{
    auto self_id = hpx::threads::get_self_id();
    thread_handle* self = ::get_thread_data(self_id);
    HPX_ASSERT(self != nullptr);
    self->retval = value_ptr;
    std::longjmp(self->env, -1);
}

///////////////////////////////////////////////////////////////////////////
hpxc_thread_t hpxc_thread_self()
{
    hpx::threads::thread_self* self = hpx::threads::get_self_ptr();
    if (self)
    {
        hpxc_thread_t t = {reinterpret_cast<void*>(self->get_thread_data())};
        return t;
    }

    hpxc_thread_t t = {nullptr};
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

int hpxc_key_create(hpxc_key_t* key, void (*destructor)(void*))
{
    if (key == nullptr)
        return EINVAL;
    try
    {
        key->handle = new tls_key(destructor);
    }
    catch (...)
    {
        key->handle = nullptr;
        return ENOMEM;
    }
    return 0;
}

int hpxc_key_delete(hpxc_key_t key)
{
    delete reinterpret_cast<tls_key*>(key.handle);
    return 0;
}

int hpxc_setspecific(hpxc_key_t key, const void* value)
{
    thread_handle* self = ::get_thread_data(hpx::threads::get_self_id());
    auto* handle = reinterpret_cast<tls_key*>(key.handle);
    if (handle == nullptr)
        return ESRCH;
    self->thread_local_storage[handle] = value;
    return 0;
}

void* hpxc_getspecific(hpxc_key_t key)
{
    thread_handle* self = ::get_thread_data(hpx::threads::get_self_id());
    auto* handle = reinterpret_cast<tls_key*>(key.handle);
    if (handle == nullptr)
        return nullptr;
    return const_cast<void*>(self->thread_local_storage[handle]);
}

int hpxc_thread_equal(hpxc_thread_t t1, hpxc_thread_t t2)
{
    return t1.handle == t2.handle;
}

void hpxc_thread_cleanup_push(void (*routine)(void*), void* arg)
{
    thread_handle* self = ::get_thread_data(hpx::threads::get_self_id());
    self->cleanup_functions.push_back(hpx::bind(routine, arg));
    return;
}

void hpxc_thread_cleanup_pop(int execute)
{
    thread_handle* self = ::get_thread_data(hpx::threads::get_self_id());
    if (execute)
    {
        self->cleanup_functions.back()();
    }
    self->cleanup_functions.pop_back();
}

#if defined(HPXC_HAVE_RW_LOCK)

int hpxc_rwlock_init(hpxc_rwlock_t* mutex, hpxc_rwlockattr_t const* attr)
{
    try
    {
        mutex->handle = new hpx::lcos::local::readers_writers_mutex();
    }
    catch (...)
    {
        mutex->handle = nullptr;
        return ENOMEM;
    }
    return 0;
}

hpxc_rwlock_t hpxc_rwmutex_alloc()
{
    hpxc_rwlock_t mtx;
    hpxc_rwlock_init(&mtx, nullptr);
    return mtx;
}

hpxc_rwlock_t lock = hpxc_rwmutex_alloc();

int hpxc_rwlock_destroy(hpxc_rwlock_t* mutex)
{
    hpx::lcos::local::readers_writers_mutex* mtx =
        reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
            mutex->handle);
    mutex->handle = nullptr;

    delete mtx;
    return 0;
}

int hpxc_rwlock_rdlock(hpxc_rwlock_t* mutex)
{
    if (mutex->handle == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
        mutex->handle);
    lock->lock_shared();
    return 0;
}

int hpxc_rwlock_timedrdlock(
    hpxc_rwlock_t* mutex, struct timespec const* abstime)
{
    return -1;
}

int hpxc_rwlock_tryrdlock(hpxc_rwlock_t* mutex)
{
    if (mutex->handle == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
        mutex->handle);
    return lock->try_lock_shared() ? 0 : EBUSY;
}

int hpxc_rwlock_wrlock(hpxc_rwlock_t* mutex)
{
    if (mutex->handle == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
        mutex->handle);
    lock->lock();
    return 0;
}

int hpxc_rwlock_timedwrlock(
    hpxc_rwlock_t* mutex, struct timespec const* abstime)
{
    return -1;
}

int hpxc_rwlock_trywrlock(hpxc_rwlock_t* mutex)
{
    if (mutex->handle == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
        mutex->handle);
    return lock->try_lock() ? 0 : EBUSY;
}

int hpxc_rwlock_unlock(hpxc_rwlock_t* mutex)
{
    if (mutex->handle == nullptr)
        return EINVAL;
    auto* lock = reinterpret_cast<hpx::lcos::local::readers_writers_mutex*>(
        mutex->handle);
    lock->unlock();
    return 0;
}
#endif

}    // extern "C"
