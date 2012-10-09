//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2011-2012 Bryce Adelstein-Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPXC_THREADS_THREAD_18_SEP_2012_0134PM)
#define HPXC_THREADS_THREAD_18_SEP_2012_0134PM

#include <hpxc/config.h>

#if defined(__cplusplus)
extern "C" {
#endif

    ///////////////////////////////////////////////////////////////////////////
    typedef struct hpxc_thread_t { void* handle; } hpxc_thread_t;
    typedef struct hpxc_mutex_t { void* handle; } hpxc_mutex_t;
    typedef struct hpxc_thread_attr_t { void* handle; } hpxc_thread_attr_t;

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
        hpxc_thread_t* thread_id,
        void** value_ptr);

    HPXC_API_EXPORT int hpxc_thread_detach(
        hpxc_thread_t* thread_id);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Terminates the calling thread.
    HPXC_API_EXPORT void hpxc_thread_exit(void* value_ptr);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Returns the ID of the calling thread.
    ///
    /// \note Returns the equivalent of \a hpx::threads::thread_invalid_id if
    ///       called from outside of an hpx-thread.
    HPXC_API_EXPORT hpxc_thread_t hpxc_thread_self(void);

	HPXC_API_EXPORT int hpxc_mutex_init(hpxc_mutex_t *mut,void *unused);
	HPXC_API_EXPORT hpxc_mutex_t hpxc_mutex_alloc();
	HPXC_API_EXPORT int hpxc_mutex_lock(hpxc_mutex_t *mut);
	HPXC_API_EXPORT int hpxc_mutex_unlock(hpxc_mutex_t *mut);
	HPXC_API_EXPORT int hpxc_mutex_trylock(hpxc_mutex_t *mut);
	HPXC_API_EXPORT void hpxc_mutex_destroy(hpxc_mutex_t *mut);
#define HPXC_MUTEX_INITIALIZER hpxc_mutex_alloc()

#if defined(__cplusplus)
}
#endif

#endif
