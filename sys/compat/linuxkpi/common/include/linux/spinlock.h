/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2017 Mellanox Technologies, Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef	_LINUX_SPINLOCK_H_
#define	_LINUX_SPINLOCK_H_

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/kernel.h>
#include <linux/rwlock.h>
#include <linux/bottom_half.h>

typedef struct {
	struct mtx m;
} spinlock_t;

#define	spin_lock(_l)		mtx_lock(&(_l)->m)
#define	spin_lock_bh(_l)	mtx_lock(&(_l)->m)
#define	spin_lock_irq(_l)	mtx_lock(&(_l)->m)
#define	spin_unlock(_l)		mtx_unlock(&(_l)->m)
#define	spin_unlock_bh(_l)	mtx_unlock(&(_l)->m)
#define	spin_unlock_irq(_l)	mtx_unlock(&(_l)->m)
#define	spin_trylock(_l)	mtx_trylock(&(_l)->m)
#define	spin_lock_nested(_l, _n) mtx_lock_flags(&(_l)->m, MTX_DUPOK)

#define	spin_lock_irqsave(lock, flags) do {	\
	(flags) = 0;				\
	spin_lock(lock);			\
} while (0)

#define	spin_lock_irqsave_nested(lock, flags, x) do {	\
	spin_lock(lock);				\
} while (0)

#define	spin_unlock_irqrestore(lock, flags) do {	\
	spin_unlock(lock);				\
} while (0)

#ifdef WITNESS_ALL
/* NOTE: the maximum WITNESS name is 64 chars */
#define	__spin_lock_name(name, file, line)		\
	(((const char *){file ":" #line "-" name}) + 	\
	(sizeof(file) > 16 ? sizeof(file) - 16 : 0))
#else
#define	__spin_lock_name(name, file, line)	name
#endif
#define	_spin_lock_name(...)		__spin_lock_name(__VA_ARGS__)
#define	spin_lock_name(name)		_spin_lock_name(name, __FILE__, __LINE__)

#define	spin_lock_init(lock)	linux_spin_lock_init(lock, spin_lock_name("lnxspin"))

static inline void
linux_spin_lock_init(spinlock_t *lock, const char *name)
{

	memset(lock, 0, sizeof(*lock));
	mtx_init(&lock->m, name, NULL, MTX_DEF | MTX_NOWITNESS);
}

static inline void
spin_lock_destroy(spinlock_t *lock)
{

       mtx_destroy(&lock->m);
}

#define	DEFINE_SPINLOCK(lock)					\
	spinlock_t lock;					\
	MTX_SYSINIT(lock, &(lock).m, spin_lock_name("lnxspin"), MTX_DEF)

#define	assert_spin_locked(_l) do {	\
	mtx_assert(&(_l)->m, MA_OWNED);	\
} while (0)

#endif					/* _LINUX_SPINLOCK_H_ */
