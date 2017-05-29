/*-
 * Copyright (c) 2017 Mark Johnston <markj@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conds
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conds, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conds and the following disclaimer in the
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/sleepqueue.h>

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

static int
add_to_sleepqueue(void *wchan, const char *wmesg, int timeout, int state)
{
	int flags, ret;

	MPASS((state & ~TASK_NORMAL) == 0);

	flags = SLEEPQ_SLEEP | ((state & TASK_INTERRUPTIBLE) != 0 ?
	    SLEEPQ_INTERRUPTIBLE : 0);

	sleepq_add(wchan, NULL, wmesg, flags, 0);
	if (timeout != 0)
		sleepq_set_timeout(wchan, timeout);
	if ((state & TASK_INTERRUPTIBLE) != 0) {
		if (timeout == 0)
			ret = sleepq_wait_sig(wchan, 0);
		else
			ret = sleepq_timedwait_sig(wchan, 0);
	} else {
		if (timeout == 0) {
			sleepq_wait(wchan, 0);
			ret = 0;
		} else
			ret = sleepq_timedwait(wchan, 0);
	}
	return (ret);
}

static int
wake_up_task(struct task_struct *task, unsigned int state)
{
	int ret;

	ret = 0;
	sleepq_lock(task);
	if ((atomic_read(&task->state) & state) != 0) {
		set_task_state(task, TASK_WAKING);
		sleepq_signal(task, SLEEPQ_SLEEP, 0, 0);
		ret = 1;
	}
	sleepq_release(task);
	return (ret);
}

bool
linux_signal_pending(struct task_struct *task)
{
	struct thread *td;
	sigset_t pending;

	td = task->task_thread;
	PROC_LOCK(td->td_proc);
	pending = td->td_siglist;
	SIGSETOR(pending, td->td_proc->p_siglist);
	SIGSETNAND(pending, td->td_sigmask);
	PROC_UNLOCK(td->td_proc);
	return (!SIGISEMPTY(pending));
}

bool
linux_fatal_signal_pending(struct task_struct *task)
{
	struct thread *td;
	bool ret;

	td = task->task_thread;
	PROC_LOCK(td->td_proc);
	ret = SIGISMEMBER(td->td_siglist, SIGKILL) ||
	    SIGISMEMBER(td->td_proc->p_siglist, SIGKILL);
	PROC_UNLOCK(td->td_proc);
	return (ret);
}

bool
linux_signal_pending_state(long state, struct task_struct *task)
{

	MPASS((state & ~TASK_NORMAL) == 0);

	if ((state & TASK_INTERRUPTIBLE) == 0)
		return (false);
	return (linux_signal_pending(task));
}

void
linux_send_sig(int signo, struct task_struct *task)
{
	struct thread *td;

	td = task->task_thread;
	PROC_LOCK(td->td_proc);
	tdsignal(td, signo);
	PROC_UNLOCK(td->td_proc);
}

int
autoremove_wake_function(wait_queue_t *wq, unsigned int state, int flags,
    void *key __unused)
{
	struct task_struct *task;
	int ret;

	task = wq->private;
	if ((ret = wake_up_task(task, state)) != 0)
		list_del_init(&wq->task_list);
	return (ret);
}

void
linux_wake_up(wait_queue_head_t *wqh, unsigned int state, int nr, bool locked)
{
	wait_queue_t *pos, *next;

	if (!locked)
		spin_lock(&wqh->lock);
	list_for_each_entry_safe(pos, next, &wqh->task_list, task_list) {
		if (((pos->func == NULL &&
		      wake_up_task(pos->private, state) != 0) ||
		     pos->func(pos, state, 0, NULL)) && --nr == 0)
			break;
	}
	if (!locked)
		spin_unlock(&wqh->lock);
}

void
linux_prepare_to_wait(wait_queue_head_t *wqh, wait_queue_t *wq, int state)
{

	spin_lock(&wqh->lock);
	if (list_empty(&wq->task_list))
		__add_wait_queue(wqh, wq);
	set_task_state(current, state);
	spin_unlock(&wqh->lock);
}

void
linux_finish_wait(wait_queue_head_t *wqh, wait_queue_t *wq)
{

	spin_lock(&wqh->lock);
	set_task_state(current, TASK_RUNNING);
	if (!list_empty(&wq->task_list)) {
		__remove_wait_queue(wqh, wq);
		INIT_LIST_HEAD(&wq->task_list);
	}
	spin_unlock(&wqh->lock);
}

bool
linux_waitqueue_active(wait_queue_head_t *wqh)
{
	bool ret;

	spin_lock(&wqh->lock);
	ret = !list_empty(&wqh->task_list);
	spin_unlock(&wqh->lock);
	return (ret);
}

long
linux_wait_event_common(wait_queue_head_t *wqh, wait_queue_t *wq, long timeoutl,
    unsigned int state, spinlock_t *lock)
{
	struct task_struct *task;
	long ret;
	int timeout;

	task = current;
	timeout = (int)timeoutl;

	if (lock != NULL)
		spin_unlock_irq(lock);

	/*
	 * Our wait queue entry is on the stack - make sure it doesn't
	 * get swapped out while we sleep.
	 */
#ifndef NO_SWAPPING
	PHOLD(task->task_thread->td_proc);
#endif
	sleepq_lock(task);
	if (atomic_read(&task->state) != TASK_WAKING) {
		ret = add_to_sleepqueue(task, "wevent", timeout, state);
	} else {
		sleepq_release(task);
		ret = 0;
	}
#ifndef NO_SWAPPING
	PRELE(task->task_thread->td_proc);
#endif

	if (lock != NULL)
		spin_lock_irq(lock);
	return (ret);
}

long
linux_schedule_timeout(long timeoutl)
{
	struct task_struct *task;
	int start, state, timeout;

	start = ticks;
	task = current;
	timeout = (int)timeoutl;
	if (timeout < 1)
		timeout = 1;
	if (timeout == MAX_SCHEDULE_TIMEOUT)
		timeout = 0;

	sleepq_lock(task);
	state = atomic_read(&task->state);
	if (state != TASK_WAKING)
		(void)add_to_sleepqueue(task, "sched", timeout, state);
	else
		sleepq_release(task);
	set_task_state(task, TASK_RUNNING);

	return (timeout == 0 ? MAX_SCHEDULE_TIMEOUT :
	    (start + timeout - ticks));
}

#define	bit_to_wchan(word, bit)	((void *)(((uintptr_t)(word) << 6) | (bit)))

void
linux_wake_up_bit(void *word, int bit)
{
	void *wchan;

	wchan = bit_to_wchan(word, bit);
	sleepq_lock(wchan);
	sleepq_signal(wchan, SLEEPQ_SLEEP, 0, 0);
	sleepq_release(wchan);
}

int
linux_wait_on_bit_timeout(unsigned long *word, int bit, unsigned int state,
    long timeoutl)
{
	struct task_struct *task;
	void *wchan;
	int ret, timeout;

	timeout = (int)timeoutl;
	if (timeout == MAX_SCHEDULE_TIMEOUT)
		timeout = 0;

	wchan = bit_to_wchan(word, bit);
	task = current;
	for (;;) {
		sleepq_lock(wchan);
		if ((*word & (1 << bit)) == 0) {
			sleepq_release(wchan);
			ret = 0;
			break;
		}
		set_task_state(task, state);
		ret = add_to_sleepqueue(wchan, "wbit", timeout, state);
		if (ret != 0)
			break;
	}
	set_task_state(task, TASK_RUNNING);
	return (ret);
}

void
linux_wake_up_atomic_t(atomic_t *a)
{

	panic("%s: unimplemented", __func__);
}

int
linux_wait_on_atomic_t(atomic_t *a, int (*cb)(atomic_t *), unsigned int state)
{

	panic("%s: unimplemented", __func__);
}

bool
linux_wake_up_state(struct task_struct *task, unsigned int state)
{

	return (wake_up_task(task, state) != 0);
}
