//  Copyright (c) 2012-2013 Alexander Duchene
//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef _PTHREAD_H
#error "pthread.h has already been included"
#endif

#ifndef _PTHREAD_H
#define _PTHREAD_H
#define _BITS_PTHREADTYPES_H

// prevent redeclaration of union/struct causing compile problems
#define __have_pthread_attr_t

#include <hpxc/threads.h>

#define pthread_t hpxc_thread_t
#define pthread_mutex_t hpxc_mutex_t
#define pthread_cond_t hpxc_cond_t
#define pthread_attr_t hpxc_thread_attr_t
#define pthread_key_t hpxc_key_t
#define pthread_spinlock_t hpxc_spinlock_t
#define pthread_rwlockattr_t hpxc_rwlockattr_t

#define pthread_create hpxc_thread_create
#define pthread_join hpxc_thread_join
#define pthread_detach hpxc_thread_detach
#define pthread_exit hpxc_thread_exit
#define pthread_self hpxc_thread_self
#define pthread_equal hpxc_thread_equal

#define pthread_attr_init hpxc_thread_attr_init
#define pthread_attr_destroy hpxc_thread_attr_destroy

#define pthread_attr_setdetachstate hpxc_thread_attr_setdetachstate
#define pthread_attr_getdetachstate hpxc_thread_attr_getdetachstate
#define PTHREAD_CREATE_DETACHED HPXC_CREATE_DETACHED
#define PTHREAD_CREATE_JOINABLE HPXC_CREATE_JOINABLE
#define PTHREAD_CANCELED HPXC_CANCELED

#define pthread_attr_setscope hpxc_thread_attr_setscope
#define pthread_attr_getscope hpxc_thread_attr_getscope
#define pthread_attr_setstacksize hpxc_thread_attr_setstacksize
#define pthread_attr_getstacksize hpxc_thread_attr_getstacksize
#define pthread_setaffinity_np hpxc_thread_setaffinity_np
#define pthread_getaffinity_np hpxc_thread_getaffinity_np

#define pthread_cond_init hpxc_cond_init
#define pthread_cond_wait hpxc_cond_wait
#define pthread_cond_signal hpxc_cond_signal
#define pthread_cond_broadcast hpxc_cond_broadcast
#define pthread_cond_destroy hpxc_cond_destroy

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_MUTEX_INITIALIZER {0}

#define pthread_mutex_init hpxc_mutex_init
#define pthread_mutex_alloc hpxc_mutex_alloc
#define pthread_mutex_lock hpxc_mutex_lock
#define pthread_mutex_unlock hpxc_mutex_unlock
#define pthread_mutex_trylock hpxc_mutex_trylock
#define pthread_mutex_destroy hpxc_mutex_destroy

#define pthread_spin_init hpxc_spin_init
#define pthread_spin_lock hpxc_spin_lock
#define pthread_spin_unlock hpxc_spin_unlock
#define pthread_spin_trylock hpxc_spin_trylock
#define pthread_spin_destroy hpxc_spin_destroy

#define pthread_rwlock_t hpxc_rwlock_t
#define pthread_rwlock_init hpxc_rwlock_init
#define pthread_rwlock_destroy hpxc_rwlock_destroy
#define pthread_rwlock_rdlock hpxc_rwlock_rdlock
#define pthread_rwlock_timedrdlock hpxc_rwlock_timedrdlock
#define pthread_rwlock_tryrdlock hpxc_rwlock_tryrdlock
#define pthread_rwlock_wrlock hpxc_rwlock_wrlock
#define pthread_rwlock_timedwrlock hpxc_rwlock_timedwrlock
#define pthread_rwlock_trywrlock hpxc_rwlock_trywrlock
#define pthread_rwlock_unlock hpxc_rwlock_unlock

#define pthread_testcancel hpxc_thread_testcancel
#define pthread_cancel hpxc_thread_cancel
#define pthread_setcancelstate hpxc_thread_setcancelstate
#define PTHREAD_CANCEL_ENABLE HPXC_THREAD_CANCEL_ENABLE
#define PTHREAD_CANCEL_DISABLE HPXC_THREAD_CANCEL_DISABLE

#define pthread_key_create hpxc_key_create
#define pthread_key_delete hpxc_key_delete
#define pthread_setspecific hpxc_setspecific
#define pthread_getspecific hpxc_getspecific

#define phtread_cleanup_push hpxc_thread_cleanup_push
#define phtread_cleanup_pop hpxc_thread_cleanup_pop

#define PTHREAD_STACK_MIN HPXC_SMALL_STACK_SIZE

#define PTHREAD_CANCEL_ASYNCHRONOUS 0
#define pthread_setcanceltype hpxc_setcanceltype

#endif
