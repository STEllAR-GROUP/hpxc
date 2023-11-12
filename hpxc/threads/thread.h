//  Copyright (c) 2007-2022 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpxc/config.h>

#if defined(__cplusplus)
#include <cstddef>
#include <ctime>
using std::size_t;
#else
#include <stddef.h>
#include <time.h>
#endif

#define HPXC_CREATE_DETACHED 1
#define HPXC_CREATE_JOINABLE 0
#define HPXC_CANCELED (void*) -1

#if defined(__cplusplus)
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
typedef struct hpxc_thread_t
{
    void* handle;
} hpxc_thread_t;

typedef struct hpxc_mutex_t
{
    void* handle;
} hpxc_mutex_t;

typedef struct hpxc_cond_t
{
    void* handle;
} hpxc_cond_t;

typedef struct hpxc_thread_attr_t
{
    void* handle;
} hpxc_thread_attr_t;

typedef struct hpxc_key_t
{
    void* handle;
} hpxc_key_t;

typedef struct hpxc_cpu_set_t
{
    void* handle;
} hpxc_cpu_set_t;

typedef struct hpxc_rwlock_t
{
    void* handle;
} hpxc_rwlock_t;

typedef struct hpxc_rwlockattr_t
{
    void* handle;
} hpxc_rwlockattr_t;

typedef hpxc_mutex_t hpxc_spinlock_t;

typedef void (*shutdown_function_t)();

///////////////////////////////////////////////////////////////////////////
/// \brief Creates a new thread of execution.
HPXC_API_EXPORT int hpxc_thread_create(hpxc_thread_t* thread_id,
    hpxc_thread_attr_t const* attributes, void* (*thread_function)(void*),
    void* arguments);

HPXC_API_EXPORT int hpxc_yield();

///////////////////////////////////////////////////////////////////////////
/// \brief Suspend the calling thread until the target thread terminates.
HPXC_API_EXPORT int hpxc_thread_join(hpxc_thread_t thread_id, void** value_ptr);

HPXC_API_EXPORT int hpxc_thread_detach(hpxc_thread_t thread_id);

HPXC_API_EXPORT void hpxc_init(void (*init_func)(), int argc, char* argv[]);

///////////////////////////////////////////////////////////////////////////
/// \brief Terminates the calling thread.
HPXC_API_EXPORT void hpxc_thread_exit(void* value_ptr);

HPXC_API_EXPORT int hpxc_thread_attr_init(hpxc_thread_attr_t* attr);
HPXC_API_EXPORT int hpxc_thread_attr_destroy(hpxc_thread_attr_t* attr);
HPXC_API_EXPORT int hpxc_thread_attr_setdetachstate(
    hpxc_thread_attr_t* attr, int detach);
HPXC_API_EXPORT int hpxc_thread_attr_getdetachstate(
    hpxc_thread_attr_t* attr, int* detach);

HPXC_API_EXPORT int hpxc_thread_getattr(
    hpxc_thread_t thread, hpxc_thread_attr_t* attr);

HPXC_API_EXPORT int hpxc_thread_attr_getstack(
    hpxc_thread_attr_t* attr, void* addr, size_t* size);

///////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_attr_setscope(
    hpxc_thread_attr_t* attr, int scope);

///////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_attr_getscope(
    hpxc_thread_attr_t* attr, int* scope);

enum
{
    HPXC_THREAD_SCOPE_SYSTEM = 0
};

///////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_attr_setstacksize(
    hpxc_thread_attr_t* attr, size_t stacksize);

///////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_attr_getstacksize(
    const hpxc_thread_attr_t* attr, size_t* stacksize);

///////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_setaffinity_np(
    hpxc_thread_t thread, size_t cpusetsize, const hpxc_cpu_set_t* cpuset);

/////////////////////////////////////////////////////////////////////////////
/// \brief Dummy function for legacy support
HPXC_API_EXPORT int hpxc_thread_getaffinity_np(
    hpxc_thread_t thread, size_t cpusetsize, hpxc_cpu_set_t* cpuset);

///////////////////////////////////////////////////////////////////////////
/// \brief Returns the ID of the calling thread.
///
/// \note Returns the equivalent of \a hpx::threads::thread_invalid_id if
///       called from outside of an hpx-thread.
HPXC_API_EXPORT hpxc_thread_t hpxc_thread_self(void);

HPXC_API_EXPORT int hpxc_cond_init(hpxc_cond_t* cond, void* unused);
HPXC_API_EXPORT int hpxc_cond_wait(hpxc_cond_t* cond, hpxc_mutex_t* unused);
HPXC_API_EXPORT int hpxc_cond_timedwait(
    hpxc_cond_t* cond, hpxc_mutex_t* mutex, const struct timespec* tm);
HPXC_API_EXPORT int hpxc_cond_signal(hpxc_cond_t* cond);
HPXC_API_EXPORT int hpxc_cond_broadcast(hpxc_cond_t* cond);
HPXC_API_EXPORT int hpxc_cond_destroy(hpxc_cond_t* cond);

HPXC_API_EXPORT int hpxc_mutex_init(hpxc_mutex_t* mut, void* unused);
HPXC_API_EXPORT hpxc_mutex_t hpxc_mutex_alloc();
HPXC_API_EXPORT int hpxc_mutex_lock(hpxc_mutex_t* mut);
HPXC_API_EXPORT int hpxc_mutex_unlock(hpxc_mutex_t* mut);
HPXC_API_EXPORT int hpxc_mutex_trylock(hpxc_mutex_t* mut);
HPXC_API_EXPORT int hpxc_mutex_destroy(hpxc_mutex_t* mut);

HPXC_API_EXPORT int hpxc_spin_init(hpxc_spinlock_t* mut, void* unused);
HPXC_API_EXPORT int hpxc_spin_lock(hpxc_spinlock_t* mut);
HPXC_API_EXPORT int hpxc_spin_unlock(hpxc_spinlock_t* mut);
HPXC_API_EXPORT int hpxc_spin_trylock(hpxc_spinlock_t* mut);
HPXC_API_EXPORT int hpxc_spin_destroy(hpxc_spinlock_t* mut);

HPXC_API_EXPORT int hpxc_thread_testcancel();
HPXC_API_EXPORT int hpxc_thread_cancel(hpxc_thread_t thread_id);

HPXC_API_EXPORT int hpxc_register_shutdown_function(shutdown_function_t f);

#if defined(HPXC_HAVE_RW_LOCK)
HPXC_API_EXPORT int hpxc_rwlock_init(
    hpxc_rwlock_t* lock, hpxc_rwlockattr_t const* attr);
HPXC_API_EXPORT extern hpxc_rwlock_t lock;
HPXC_API_EXPORT int hpxc_rwlock_destroy(hpxc_rwlock_t* lock);
HPXC_API_EXPORT int hpxc_rwlock_rdlock(hpxc_rwlock_t* lock);
HPXC_API_EXPORT int hpxc_rwlock_timedrdlock(
    hpxc_rwlock_t* lock, struct timespec const* abstime);
HPXC_API_EXPORT int hpxc_rwlock_tryrdlock(hpxc_rwlock_t* lock);
HPXC_API_EXPORT int hpxc_rwlock_wrlock(hpxc_rwlock_t* lock);
HPXC_API_EXPORT int hpxc_rwlock_timedwrlock(
    hpxc_rwlock_t* lock, struct timespec const* abstime);
HPXC_API_EXPORT int hpxc_rwlock_trywrlock(hpxc_rwlock_t* lock);
HPXC_API_EXPORT int hpxc_rwlock_unlock(hpxc_rwlock_t* lock);
#endif

enum
{
    HPXC_THREAD_CANCELED = 1,
    HPXC_THREAD_CANCEL_ENABLE = 2,
    HPXC_THREAD_CANCEL_DISABLE = 0,
    HPXC_THREAD_CANCEL_DEFERRED = 0,
    HPXC_THREAD_CANCEL_ASYNCHRONOUS = 8,
    HPXC_THREAD_IS_DETACHED = 16
};

HPXC_API_EXPORT int hpxc_thread_setcancelstate(int state, int* old_state);
HPXC_API_EXPORT int hpxc_thread_setcanceltype(int state, int* old_state);

///////////////////////////////////////////////////////////////////////////
/// \brief Creates key for thread local storage usable by all threads
HPXC_API_EXPORT int hpxc_key_create(hpxc_key_t* key, void (*destructor)(void*));

///////////////////////////////////////////////////////////////////////////:
/// \brief Deletes a key
HPXC_API_EXPORT int hpxc_key_delete(hpxc_key_t key);

///////////////////////////////////////////////////////////////////////////
/// \brief Associates a value with a key
HPXC_API_EXPORT int hpxc_setspecific(hpxc_key_t key, const void* value);

///////////////////////////////////////////////////////////////////////////
/// \brief Associates a value with a key
HPXC_API_EXPORT void* hpxc_getspecific(hpxc_key_t key);

HPXC_API_EXPORT int hpxc_thread_equal(hpxc_thread_t t1, hpxc_thread_t t2);

HPXC_API_EXPORT void hpxc_thread_cleanup_push(
    void (*routine)(void*), void* arg);
HPXC_API_EXPORT void hpxc_thread_cleanup_pop(int execute);

#define HPXC_MUTEX_INITIALIZER hpxc_mutex_alloc()

#if defined(__cplusplus)
}
#endif
