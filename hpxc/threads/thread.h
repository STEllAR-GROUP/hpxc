//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//  Copyright (c) 2012-2013 Alexander Duchene
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPXC_THREADS_THREAD_18_SEP_2012_0134PM)
#define HPXC_THREADS_THREAD_18_SEP_2012_0134PM

#include <hpxc/config.h>
#if defined(__cplusplus)
#include <cstddef>
using std::size_t;
#else
#include <stddef.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

    ///////////////////////////////////////////////////////////////////////////
    typedef struct hpxc_thread_t { void *handle; } hpxc_thread_t;
    typedef struct hpxc_mutex_t { void* handle; } hpxc_mutex_t;
    typedef struct hpxc_cond_t { void* handle; } hpxc_cond_t;
    typedef struct hpxc_thread_attr_t { void* handle; } hpxc_thread_attr_t;
    typedef struct hpxc_key_t {void* handle;} hpxc_key_t;
    typedef struct hpxc_cpu_set_t {void* handle;} hpxc_cpu_set_t;
    typedef hpxc_mutex_t hpxc_spinlock_t;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Creates a new thread of execution.
    HPXC_API_EXPORT int hpxc_thread_create(
        hpxc_thread_t* thread_id, 
        hpxc_thread_attr_t const* attributes,
        void* (*thread_function)(void*), 
        void* arguments);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Suspend the calling thread until the target thread terminates.
    HPXC_API_EXPORT int hpxc_thread_join(
        hpxc_thread_t thread_id,
        void** value_ptr);

    HPXC_API_EXPORT int hpxc_thread_detach(
        hpxc_thread_t thread_id);

    HPXC_API_EXPORT void hpxc_init(
        void (*init_func)(),
        int argc,
        char *argv[]
        );

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Terminates the calling thread.
    HPXC_API_EXPORT void hpxc_thread_exit(void* value_ptr);

    HPXC_API_EXPORT int hpxc_thread_attr_init(hpxc_thread_attr_t *attr);
    HPXC_API_EXPORT int hpxc_thread_attr_destroy(hpxc_thread_attr_t *attr);
    HPXC_API_EXPORT int hpxc_thread_attr_setdetachstate(
        hpxc_thread_attr_t *attr,int detach);
    HPXC_API_EXPORT int hpxc_thread_attr_getdetachstate(
        hpxc_thread_attr_t *attr,int *detach);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_attr_setscope(hpxc_thread_attr_t *attr, int scope);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_attr_getscope(hpxc_thread_attr_t *attr, int *scope);
    enum {
        HPXC_THREAD_SCOPE_SYSTEM=0
    };
    
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_attr_setstacksize(hpxc_thread_attr_t *attr, size_t stacksize);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_attr_getstacksize(const hpxc_thread_attr_t *attr, size_t* stacksize);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_setaffinity_np(hpxc_thread_t thread, size_t cpusetsize, const hpxc_cpu_set_t *cpuset);


    /////////////////////////////////////////////////////////////////////////////
    /// \brief Dummy function for legacy support
    HPXC_API_EXPORT int hpxc_thread_getaffinity_np(hpxc_thread_t thread, size_t cpusetsize, hpxc_cpu_set_t *cpuset);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Returns the ID of the calling thread.
    ///
    /// \note Returns the equivalent of \a hpx::threads::thread_invalid_id if
    ///       called from outside of an hpx-thread.
    HPXC_API_EXPORT hpxc_thread_t hpxc_thread_self(void);

    HPXC_API_EXPORT int hpxc_cond_init(hpxc_cond_t *cond,void *unused);
    HPXC_API_EXPORT int hpxc_cond_wait(hpxc_cond_t *cond,hpxc_mutex_t *unused);
    HPXC_API_EXPORT int hpxc_cond_signal(hpxc_cond_t *cond);
    HPXC_API_EXPORT int hpxc_cond_broadcast(hpxc_cond_t *cond);

    HPXC_API_EXPORT int hpxc_mutex_init(hpxc_mutex_t *mut,void *unused);
    HPXC_API_EXPORT hpxc_mutex_t hpxc_mutex_alloc();
    HPXC_API_EXPORT int hpxc_mutex_lock(hpxc_mutex_t *mut);
    HPXC_API_EXPORT int hpxc_mutex_unlock(hpxc_mutex_t *mut);
    HPXC_API_EXPORT int hpxc_mutex_trylock(hpxc_mutex_t *mut);
    HPXC_API_EXPORT int  hpxc_mutex_destroy(hpxc_mutex_t *mut);
    
    HPXC_API_EXPORT int hpxc_spin_init(hpxc_spinlock_t *mut,void *unused);
    HPXC_API_EXPORT int hpxc_spin_lock(hpxc_spinlock_t *mut);
    HPXC_API_EXPORT int hpxc_spin_unlock(hpxc_spinlock_t *mut);
    HPXC_API_EXPORT int hpxc_spin_trylock(hpxc_spinlock_t *mut);
    HPXC_API_EXPORT int hpxc_spin_destroy(hpxc_spinlock_t *mut);

    HPXC_API_EXPORT int hpxc_thread_testcancel();
    HPXC_API_EXPORT int hpxc_thread_cancel(hpxc_thread_t thread_id);
    enum {
        HPXC_THREAD_CANCELED=1,
        HPXC_THREAD_CANCEL_ENABLE=2,
        HPXC_THREAD_CANCEL_DISABLE=0,
        HPXC_THREAD_CANCEL_DEFERRED=0,
        HPXC_THREAD_CANCEL_ASYNCHRONOUS=8
    };
    HPXC_API_EXPORT int hpxc_thread_setcancelstate(int state,int *old_state);
    HPXC_API_EXPORT int hpxc_thread_setcanceltype(int state,int *old_state);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Creates key for thread local storage usable by all threads
    HPXC_API_EXPORT int hpxc_key_create(hpxc_key_t *key, void (*destructor)(void*));


    ///////////////////////////////////////////////////////////////////////////:
    /// \brief Deletes a key
    HPXC_API_EXPORT int hpxc_key_delete(hpxc_key_t key);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Associates a value with a key
    HPXC_API_EXPORT int hpxc_setspecific(hpxc_key_t key, const void *value);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Associates a value with a key
    HPXC_API_EXPORT void* hpxc_getspecific(hpxc_key_t key);

#define HPXC_MUTEX_INITIALIZER hpxc_mutex_alloc()

#if defined(__cplusplus)
}
#endif

#endif
