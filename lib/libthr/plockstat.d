/*
 * Copyright (c) 2010 The FreeBSD Foundation 
 * Copyright (c) 2020 Dell
 * All rights reserved. 
 * 
 * This software was developed by Rui Paulo under sponsorship from the
 * FreeBSD Foundation. 
 *  
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * $FreeBSD$
 */
#include <sys/_pthreadtypes.h>

provider plockstat {
	/**
	 * Hold event probe.
	 * Fires immediately after a mutex is acquired
	 *
	 * is_recursive: a boolean value indicating if the acquisition
	 *     was recursive on a recursive mutex
	 * cntspin: number of spin iterations that the acquiring
	 *     thread spent on this mutex
	 * cntyield: number of yield iterations that the acquiring
	 *     thread spent on this mutex
	 */
	probe mutex__acquire(pthread_mutex_t mutex, int is_recursive, int cntspin, int cntyield);

	/**
	 * Contention event probe.
	 * Fires before a thread blocks on a held mutex.  Both
	 * mutex-block, mutex-spin, mutex-yield might fire for
	 * a single lock acquisition.
	 */
	probe mutex__block(pthread_mutex_t mutex);

	/**
	 * Contention event probe.
	 * Fires before a thread begins spinning on a held mutex.  Both
	 * mutex-block, mutex-spin, mutex-yield might fire for
	 * a single lock acquisition.
	 */
	probe mutex__spin(pthread_mutex_t mutex);

	/**
	 * Contention event probe.
	 * Fires before a thread begins yield on a held mutex.  Both
	 * mutex-block, mutex-spin, mutex-yield might fire for
	 * a single lock acquisition.
	 */
	probe mutex__yield(pthread_mutex_t mutex);

	/**
	 * Hold event probe.
	 * Fires immediately after a mutex is released.
	 *
	 * is_recursive: a boolean value indicating if the event
	 *     corresponds to a recursive release on a
	 *     recursive mutex.
	 */
	probe mutex__release(pthread_mutex_t mutex, int is_recursive);

	/**
	 * Mutex handler mapping probe.
	 * Collect mapping from internal to external pointer.
	 */
	probe mutex__map(pthread_mutex_t mtx_int, void* mtx_ext);

	/**
	 * Hold event probe.
	 * Fires immediately after an rwlock is acquired.
	 */
	probe rwlock__acquire(pthread_rwlock_t rwlock, int is_writer);

	/**
	 * Contention event probe.
	 * Fires before a thread blocks while attempting to acquire a
	 * lock.
	 */
	probe rwlock__block(pthread_rwlock_t rwlock, int is_writer);

	/**
	 * Hold event probe.
	 * Fires immediately after a lock is released.
	 */
	probe rwlock__release(pthread_rwlock_t rwlock, int is_writer);

	/**
	 * rwlock handler mapping probe.
	 * Collect mapping from internal to external pointer.
	 */
	probe rwlock__map(pthread_rwlock_t rwl_int, void* rwl_ext);

	/**
	 * Hold event probe.
	 * Fires immediately after a spinlock is acquired
	 * cntspin - number of spins after last yield
	 * cntyield - number of yields, thread is yielding every 100000 spins
	 * so total number of spins before acquire is 100000 * cntyield + cntspin
	 */
	probe spinlock__acquire(pthread_spinlock_t spinlock, int cntspin, int cntyield);

	/**
	 * Contention event probe.
	 * Fires before a thread blocks in a spin loop
	 */
	probe spinlock__block(pthread_spinlock_t spinlock);

	/**
	 * Hold event probe.
	 * Fires immediately after a spinlock is released
	 */
	probe spinlock__release(pthread_spinlock_t spinlock);

	/**
	 * spinlock handler mapping probe.
	 * Collect mapping from internal to external pointer.
	 */
	probe spinlock__map(pthread_spinlock_t sl_int, void* sl_ext);
};
