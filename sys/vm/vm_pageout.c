/*-
 * Copyright (c) 1991 Regents of the University of California.
 * All rights reserved.
 * Copyright (c) 1994 John S. Dyson
 * All rights reserved.
 * Copyright (c) 1994 David Greenman
 * All rights reserved.
 * Copyright (c) 2005 Yahoo! Technologies Norway AS
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)vm_pageout.c	7.4 (Berkeley) 5/7/91
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Authors: Avadis Tevanian, Jr., Michael Wayne Young
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 *	The proverbial page-out daemon.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_vm.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/eventhandler.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/ktr.h>
#include <sys/mount.h>
#include <sys/racct.h>
#include <sys/resourcevar.h>
#include <sys/sched.h>
#include <sys/sdt.h>
#include <sys/signalvar.h>
#include <sys/smp.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/vmmeter.h>
#include <sys/rwlock.h>
#include <sys/sx.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_pageout.h>
#include <vm/vm_pager.h>
#include <vm/vm_phys.h>
#include <vm/swap_pager.h>
#include <vm/vm_extern.h>
#include <vm/uma.h>

/*
 * System initialization
 */

/* the kernel process "vm_pageout"*/
static void vm_pageout(void);
static void vm_pageout_init(void);
static int vm_pageout_clean(vm_page_t m, int *numpagedout);
static int vm_pageout_cluster(vm_page_t m);
static void vm_pageout_scan(struct vm_domain *vmd, int pass);
static void vm_pageout_mightbe_oom(struct vm_domain *vmd, int page_shortage,
    int starting_page_shortage);

SYSINIT(pagedaemon_init, SI_SUB_KTHREAD_PAGE, SI_ORDER_FIRST, vm_pageout_init,
    NULL);

struct proc *pageproc;

static struct kproc_desc page_kp = {
	"pagedaemon",
	vm_pageout,
	&pageproc
};
SYSINIT(pagedaemon, SI_SUB_KTHREAD_PAGE, SI_ORDER_SECOND, kproc_start,
    &page_kp);

SDT_PROVIDER_DEFINE(vm);
SDT_PROBE_DEFINE(vm, , , vm__lowmem_cache);
SDT_PROBE_DEFINE(vm, , , vm__lowmem_scan);

#if !defined(NO_SWAPPING)
/* the kernel process "vm_daemon"*/
static void vm_daemon(void);
static struct	proc *vmproc;

static struct kproc_desc vm_kp = {
	"vmdaemon",
	vm_daemon,
	&vmproc
};
SYSINIT(vmdaemon, SI_SUB_KTHREAD_VM, SI_ORDER_FIRST, kproc_start, &vm_kp);
#endif

/* Sleep intervals for pagedaemon threads, in subdivisions of one second. */
#define	VM_LAUNDER_INTERVAL	10
#define	VM_INACT_SCAN_INTERVAL	2

#define	VM_LAUNDER_RATE		(VM_LAUNDER_INTERVAL / VM_INACT_SCAN_INTERVAL)

int vm_pages_needed;		/* Event on which pageout daemon sleeps */
int vm_pageout_deficit;		/* Estimated number of pages deficit */
int vm_pageout_wakeup_thresh;
static int vm_pageout_oom_seq = 12;

#if !defined(NO_SWAPPING)
static int vm_pageout_req_swapout;	/* XXX */
static int vm_daemon_needed;
static struct mtx vm_daemon_mtx;
/* Allow for use by vm_pageout before vm_daemon is initialized. */
MTX_SYSINIT(vm_daemon, &vm_daemon_mtx, "vm daemon", MTX_DEF);
#endif
static int vm_pageout_update_period;
static int disable_swap_pageouts;
static int lowmem_period = 10;
static time_t lowmem_uptime;

#if defined(NO_SWAPPING)
static int vm_swap_enabled = 0;
static int vm_swap_idle_enabled = 0;
#else
static int vm_swap_enabled = 1;
static int vm_swap_idle_enabled = 0;
#endif
static int vm_swapdev_cnt = 0;

static int vm_panic_on_oom = 0;

SYSCTL_INT(_vm, OID_AUTO, panic_on_oom,
	CTLFLAG_RWTUN, &vm_panic_on_oom, 0,
	"panic on out of memory instead of killing the largest process");

SYSCTL_INT(_vm, OID_AUTO, pageout_wakeup_thresh,
	CTLFLAG_RW, &vm_pageout_wakeup_thresh, 0,
	"free page threshold for waking up the pageout daemon");

SYSCTL_INT(_vm, OID_AUTO, pageout_update_period,
	CTLFLAG_RW, &vm_pageout_update_period, 0,
	"Maximum active LRU update period");
  
SYSCTL_INT(_vm, OID_AUTO, lowmem_period, CTLFLAG_RW, &lowmem_period, 0,
	"Low memory callback period");

#if defined(NO_SWAPPING)
SYSCTL_INT(_vm, VM_SWAPPING_ENABLED, swap_enabled,
	CTLFLAG_RD, &vm_swap_enabled, 0, "Enable entire process swapout");
SYSCTL_INT(_vm, OID_AUTO, swap_idle_enabled,
	CTLFLAG_RD, &vm_swap_idle_enabled, 0, "Allow swapout on idle criteria");
#else
SYSCTL_INT(_vm, VM_SWAPPING_ENABLED, swap_enabled,
	CTLFLAG_RW, &vm_swap_enabled, 0, "Enable entire process swapout");
SYSCTL_INT(_vm, OID_AUTO, swap_idle_enabled,
	CTLFLAG_RW, &vm_swap_idle_enabled, 0, "Allow swapout on idle criteria");
#endif

SYSCTL_INT(_vm, OID_AUTO, disable_swapspace_pageouts,
	CTLFLAG_RW, &disable_swap_pageouts, 0, "Disallow swapout of dirty pages");

static int pageout_lock_miss;
SYSCTL_INT(_vm, OID_AUTO, pageout_lock_miss,
	CTLFLAG_RD, &pageout_lock_miss, 0, "vget() lock misses during pageout");

SYSCTL_INT(_vm, OID_AUTO, pageout_oom_seq,
	CTLFLAG_RW, &vm_pageout_oom_seq, 0,
	"back-to-back calls to oom detector to start OOM");

static int act_scan_laundry_weight = 3;
SYSCTL_INT(_vm, OID_AUTO, act_scan_laundry_weight,
	CTLFLAG_RW, &act_scan_laundry_weight, 0,
	"weight given to clean vs. dirty pages in active queue scans");

static int idle_launder_rate = 1 * VM_LAUNDER_INTERVAL;
SYSCTL_INT(_vm, OID_AUTO, idle_launder_rate,
	CTLFLAG_RWTUN, &idle_launder_rate, 0,
	"desired number of idle page launderings per second");

#define VM_PAGEOUT_PAGE_COUNT 16
int vm_pageout_page_count = VM_PAGEOUT_PAGE_COUNT;

int vm_page_max_wired;		/* XXX max # of wired pages system-wide */
SYSCTL_INT(_vm, OID_AUTO, max_wired,
	CTLFLAG_RW, &vm_page_max_wired, 0, "System-wide limit to wired page count");

static boolean_t vm_pageout_fallback_object_lock(vm_page_t, vm_page_t *);
static void vm_pageout_launder(struct vm_domain *vmd);
static void vm_pageout_laundry_worker(void *arg);
#if !defined(NO_SWAPPING)
static void vm_pageout_map_deactivate_pages(vm_map_t, long);
static void vm_pageout_object_deactivate_pages(pmap_t, vm_object_t, long);
static void vm_req_vmdaemon(int req);
#endif
static boolean_t vm_pageout_page_lock(vm_page_t, vm_page_t *);

/*
 * Initialize a dummy page for marking the caller's place in the specified
 * paging queue.  In principle, this function only needs to set the flag
 * PG_MARKER.  Nonetheless, it wirte busies and initializes the hold count
 * to one as safety precautions.
 */ 
static void
vm_pageout_init_marker(vm_page_t marker, u_short queue)
{

	bzero(marker, sizeof(*marker));
	marker->flags = PG_MARKER;
	marker->busy_lock = VPB_SINGLE_EXCLUSIVER;
	marker->queue = queue;
	marker->hold_count = 1;
}

/*
 * vm_pageout_fallback_object_lock:
 * 
 * Lock vm object currently associated with `m'. VM_OBJECT_TRYWLOCK is
 * known to have failed and page queue must be either PQ_ACTIVE or
 * PQ_INACTIVE.  To avoid lock order violation, unlock the page queues
 * while locking the vm object.  Use marker page to detect page queue
 * changes and maintain notion of next page on page queue.  Return
 * TRUE if no changes were detected, FALSE otherwise.  vm object is
 * locked on return.
 * 
 * This function depends on both the lock portion of struct vm_object
 * and normal struct vm_page being type stable.
 */
static boolean_t
vm_pageout_fallback_object_lock(vm_page_t m, vm_page_t *next)
{
	struct vm_page marker;
	struct vm_pagequeue *pq;
	boolean_t unchanged;
	u_short queue;
	vm_object_t object;

	queue = m->queue;
	vm_pageout_init_marker(&marker, queue);
	pq = vm_page_pagequeue(m);
	object = m->object;
	
	TAILQ_INSERT_AFTER(&pq->pq_pl, m, &marker, plinks.q);
	vm_pagequeue_unlock(pq);
	vm_page_unlock(m);
	VM_OBJECT_WLOCK(object);
	vm_page_lock(m);
	vm_pagequeue_lock(pq);

	/*
	 * The page's object might have changed, and/or the page might
	 * have moved from its original position in the queue.  If the
	 * page's object has changed, then the caller should abandon
	 * processing the page because the wrong object lock was
	 * acquired.  Use the marker's plinks.q, not the page's, to
	 * determine if the page has been moved.  The state of the
	 * page's plinks.q can be indeterminate; whereas, the marker's
	 * plinks.q must be valid.
	 */
	*next = TAILQ_NEXT(&marker, plinks.q);
	unchanged = m->object == object &&
	    m == TAILQ_PREV(&marker, pglist, plinks.q);
	KASSERT(!unchanged || m->queue == queue,
	    ("page %p queue %d %d", m, queue, m->queue));
	TAILQ_REMOVE(&pq->pq_pl, &marker, plinks.q);
	return (unchanged);
}

/*
 * Lock the page while holding the page queue lock.  Use marker page
 * to detect page queue changes and maintain notion of next page on
 * page queue.  Return TRUE if no changes were detected, FALSE
 * otherwise.  The page is locked on return. The page queue lock might
 * be dropped and reacquired.
 *
 * This function depends on normal struct vm_page being type stable.
 */
static boolean_t
vm_pageout_page_lock(vm_page_t m, vm_page_t *next)
{
	struct vm_page marker;
	struct vm_pagequeue *pq;
	boolean_t unchanged;
	u_short queue;

	vm_page_lock_assert(m, MA_NOTOWNED);
	if (vm_page_trylock(m))
		return (TRUE);

	queue = m->queue;
	vm_pageout_init_marker(&marker, queue);
	pq = vm_page_pagequeue(m);

	TAILQ_INSERT_AFTER(&pq->pq_pl, m, &marker, plinks.q);
	vm_pagequeue_unlock(pq);
	vm_page_lock(m);
	vm_pagequeue_lock(pq);

	/* Page queue might have changed. */
	*next = TAILQ_NEXT(&marker, plinks.q);
	unchanged = m == TAILQ_PREV(&marker, pglist, plinks.q);
	KASSERT(!unchanged || m->queue == queue,
	    ("page %p queue %d %d", m, queue, m->queue));
	TAILQ_REMOVE(&pq->pq_pl, &marker, plinks.q);
	return (unchanged);
}

/*
 * vm_pageout_clean:
 *
 * Clean the page and remove it from the laundry.
 * 
 * We set the busy bit to cause potential page faults on this page to
 * block.  Note the careful timing, however, the busy bit isn't set till
 * late and we cannot do anything that will mess with the page.
 */
static int
vm_pageout_cluster(vm_page_t m)
{
	vm_object_t object;
	vm_page_t mc[2*vm_pageout_page_count], pb, ps;
	int pageout_count;
	int ib, is, page_base;
	vm_pindex_t pindex = m->pindex;

	vm_page_lock_assert(m, MA_OWNED);
	object = m->object;
	VM_OBJECT_ASSERT_WLOCKED(object);

	/*
	 * It doesn't cost us anything to pageout OBJT_DEFAULT or OBJT_SWAP
	 * with the new swapper, but we could have serious problems paging
	 * out other object types if there is insufficient memory.  
	 *
	 * Unfortunately, checking free memory here is far too late, so the
	 * check has been moved up a procedural level.
	 */

	/*
	 * Can't clean the page if it's busy or held.
	 */
	vm_page_assert_unbusied(m);
	KASSERT(m->hold_count == 0, ("vm_pageout_clean: page %p is held", m));
	vm_page_unlock(m);

	mc[vm_pageout_page_count] = pb = ps = m;
	pageout_count = 1;
	page_base = vm_pageout_page_count;
	ib = 1;
	is = 1;

	/*
	 * Scan object for clusterable pages.
	 *
	 * We can cluster ONLY if: ->> the page is NOT
	 * clean, wired, busy, held, or mapped into a
	 * buffer, and one of the following:
	 * 1) The page is in the laundry.
	 * -or-
	 * 2) we force the issue.
	 *
	 * During heavy mmap/modification loads the pageout
	 * daemon can really fragment the underlying file
	 * due to flushing pages out of order and not trying
	 * align the clusters (which leave sporatic out-of-order
	 * holes).  To solve this problem we do the reverse scan
	 * first and attempt to align our cluster, then do a 
	 * forward scan if room remains.
	 */
more:
	while (ib && pageout_count < vm_pageout_page_count) {
		vm_page_t p;

		if (ib > pindex) {
			ib = 0;
			break;
		}

		if ((p = vm_page_prev(pb)) == NULL || vm_page_busied(p)) {
			ib = 0;
			break;
		}
		vm_page_test_dirty(p);
		if (p->dirty == 0) {
			ib = 0;
			break;
		}
		vm_page_lock(p);
		if ((p->queue != PQ_LAUNDRY && p->queue != PQ_STASIS) ||
		    p->hold_count != 0) {	/* may be undergoing I/O */
			vm_page_unlock(p);
			ib = 0;
			break;
		}
		vm_page_unlock(p);
		mc[--page_base] = pb = p;
		++pageout_count;
		++ib;
		/*
		 * alignment boundry, stop here and switch directions.  Do
		 * not clear ib.
		 */
		if ((pindex - (ib - 1)) % vm_pageout_page_count == 0)
			break;
	}

	while (pageout_count < vm_pageout_page_count && 
	    pindex + is < object->size) {
		vm_page_t p;

		if ((p = vm_page_next(ps)) == NULL || vm_page_busied(p))
			break;
		vm_page_test_dirty(p);
		if (p->dirty == 0)
			break;
		vm_page_lock(p);
		if (p->queue != PQ_LAUNDRY ||
		    p->hold_count != 0) {	/* may be undergoing I/O */
			vm_page_unlock(p);
			break;
		}
		vm_page_unlock(p);
		mc[page_base + pageout_count] = ps = p;
		++pageout_count;
		++is;
	}

	/*
	 * If we exhausted our forward scan, continue with the reverse scan
	 * when possible, even past a page boundry.  This catches boundry
	 * conditions.
	 */
	if (ib && pageout_count < vm_pageout_page_count)
		goto more;

	/*
	 * we allow reads during pageouts...
	 */
	return (vm_pageout_flush(&mc[page_base], pageout_count, 0, 0, NULL,
	    NULL));
}

/*
 * vm_pageout_flush() - launder the given pages
 *
 *	The given pages are laundered.  Note that we setup for the start of
 *	I/O ( i.e. busy the page ), mark it read-only, and bump the object
 *	reference count all in here rather then in the parent.  If we want
 *	the parent to do more sophisticated things we may have to change
 *	the ordering.
 *
 *	Returned runlen is the count of pages between mreq and first
 *	page after mreq with status VM_PAGER_AGAIN.
 *	*eio is set to TRUE if pager returned VM_PAGER_ERROR or VM_PAGER_FAIL
 *	for any page in runlen set.
 */
int
vm_pageout_flush(vm_page_t *mc, int count, int flags, int mreq, int *prunlen,
    boolean_t *eio)
{
	vm_object_t object = mc[0]->object;
	int pageout_status[count];
	int numpagedout = 0;
	int i, runlen;

	VM_OBJECT_ASSERT_WLOCKED(object);

	/*
	 * Initiate I/O.  Bump the vm_page_t->busy counter and
	 * mark the pages read-only.
	 *
	 * We do not have to fixup the clean/dirty bits here... we can
	 * allow the pager to do it after the I/O completes.
	 *
	 * NOTE! mc[i]->dirty may be partial or fragmented due to an
	 * edge case with file fragments.
	 */
	for (i = 0; i < count; i++) {
		KASSERT(mc[i]->valid == VM_PAGE_BITS_ALL,
		    ("vm_pageout_flush: partially invalid page %p index %d/%d",
			mc[i], i, count));
		vm_page_sbusy(mc[i]);
		pmap_remove_write(mc[i]);
	}
	vm_object_pip_add(object, count);

	vm_pager_put_pages(object, mc, count, flags, pageout_status);

	runlen = count - mreq;
	if (eio != NULL)
		*eio = FALSE;
	for (i = 0; i < count; i++) {
		vm_page_t mt = mc[i];

		KASSERT(pageout_status[i] == VM_PAGER_PEND ||
		    !pmap_page_is_write_mapped(mt),
		    ("vm_pageout_flush: page %p is not write protected", mt));
		switch (pageout_status[i]) {
		case VM_PAGER_OK:
		case VM_PAGER_PEND:
			numpagedout++;
			break;
		case VM_PAGER_BAD:
			/*
			 * Page outside of range of object. Right now we
			 * essentially lose the changes by pretending it
			 * worked.
			 */
			vm_page_undirty(mt);
			vm_page_lock(mt);
			vm_page_deactivate(mt);
			vm_page_unlock(mt);
			break;
		case VM_PAGER_ERROR:
		case VM_PAGER_FAIL:
			/*
			 * If we've run out of swap space or there are no swap
			 * devices to begin with, place the page in stasis.  If
			 * swap devices are available, we will periodically
			 * re-attempt a pageout.
			 *
			 * For other types of errors, reactivate the page to
			 * avoid clogging the laundry queue.  This ensures that
			 * the pagedaemon will continue aggressively scanning
			 * the active queue if necessary.
			 */
			vm_page_lock(mt);
			if (pageout_status[i] == VM_PAGER_FAIL &&
			    mt->dirty != 0 && (object->type == OBJT_DEFAULT ||
			    object->type == OBJT_SWAP))
				vm_page_enter_stasis(mt);
			else
				vm_page_activate(mt);
			vm_page_unlock(mt);
			if (eio != NULL && i >= mreq && i - mreq < runlen)
				*eio = TRUE;
			break;
		case VM_PAGER_AGAIN:
			if (i >= mreq && i - mreq < runlen)
				runlen = i - mreq;
			break;
		}

		/*
		 * If the operation is still going, leave the page busy to
		 * block all other accesses. Also, leave the paging in
		 * progress indicator set so that we don't attempt an object
		 * collapse.
		 */
		if (pageout_status[i] != VM_PAGER_PEND) {
			vm_object_pip_wakeup(object);
			vm_page_sunbusy(mt);
		}
	}
	if (prunlen != NULL)
		*prunlen = runlen;
	return (numpagedout);
}

#if !defined(NO_SWAPPING)
/*
 *	vm_pageout_object_deactivate_pages
 *
 *	Deactivate enough pages to satisfy the inactive target
 *	requirements.
 *
 *	The object and map must be locked.
 */
static void
vm_pageout_object_deactivate_pages(pmap_t pmap, vm_object_t first_object,
    long desired)
{
	vm_object_t backing_object, object;
	vm_page_t p;
	int act_delta, remove_mode;

	VM_OBJECT_ASSERT_LOCKED(first_object);
	if ((first_object->flags & OBJ_FICTITIOUS) != 0)
		return;
	for (object = first_object;; object = backing_object) {
		if (pmap_resident_count(pmap) <= desired)
			goto unlock_return;
		VM_OBJECT_ASSERT_LOCKED(object);
		if ((object->flags & OBJ_UNMANAGED) != 0 ||
		    object->paging_in_progress != 0)
			goto unlock_return;

		remove_mode = 0;
		if (object->shadow_count > 1)
			remove_mode = 1;
		/*
		 * Scan the object's entire memory queue.
		 */
		TAILQ_FOREACH(p, &object->memq, listq) {
			if (pmap_resident_count(pmap) <= desired)
				goto unlock_return;
			if (vm_page_busied(p))
				continue;
			PCPU_INC(cnt.v_pdpages);
			vm_page_lock(p);
			if (p->wire_count != 0 || p->hold_count != 0 ||
			    !pmap_page_exists_quick(pmap, p)) {
				vm_page_unlock(p);
				continue;
			}
			act_delta = pmap_ts_referenced(p);
			if ((p->aflags & PGA_REFERENCED) != 0) {
				if (act_delta == 0)
					act_delta = 1;
				vm_page_aflag_clear(p, PGA_REFERENCED);
			}
			if (p->queue != PQ_ACTIVE && act_delta != 0) {
				vm_page_activate(p);
				p->act_count += act_delta;
			} else if (p->queue == PQ_ACTIVE) {
				if (act_delta == 0) {
					p->act_count -= min(p->act_count,
					    ACT_DECLINE);
					if (!remove_mode && p->act_count == 0) {
						pmap_remove_all(p);
						vm_page_deactivate(p);
					} else
						vm_page_requeue(p);
				} else {
					vm_page_activate(p);
					if (p->act_count < ACT_MAX -
					    ACT_ADVANCE)
						p->act_count += ACT_ADVANCE;
					vm_page_requeue(p);
				}
			} else if (p->queue == PQ_INACTIVE)
				pmap_remove_all(p);
			vm_page_unlock(p);
		}
		if ((backing_object = object->backing_object) == NULL)
			goto unlock_return;
		VM_OBJECT_RLOCK(backing_object);
		if (object != first_object)
			VM_OBJECT_RUNLOCK(object);
	}
unlock_return:
	if (object != first_object)
		VM_OBJECT_RUNLOCK(object);
}

/*
 * deactivate some number of pages in a map, try to do it fairly, but
 * that is really hard to do.
 */
static void
vm_pageout_map_deactivate_pages(map, desired)
	vm_map_t map;
	long desired;
{
	vm_map_entry_t tmpe;
	vm_object_t obj, bigobj;
	int nothingwired;

	if (!vm_map_trylock(map))
		return;

	bigobj = NULL;
	nothingwired = TRUE;

	/*
	 * first, search out the biggest object, and try to free pages from
	 * that.
	 */
	tmpe = map->header.next;
	while (tmpe != &map->header) {
		if ((tmpe->eflags & MAP_ENTRY_IS_SUB_MAP) == 0) {
			obj = tmpe->object.vm_object;
			if (obj != NULL && VM_OBJECT_TRYRLOCK(obj)) {
				if (obj->shadow_count <= 1 &&
				    (bigobj == NULL ||
				     bigobj->resident_page_count < obj->resident_page_count)) {
					if (bigobj != NULL)
						VM_OBJECT_RUNLOCK(bigobj);
					bigobj = obj;
				} else
					VM_OBJECT_RUNLOCK(obj);
			}
		}
		if (tmpe->wired_count > 0)
			nothingwired = FALSE;
		tmpe = tmpe->next;
	}

	if (bigobj != NULL) {
		vm_pageout_object_deactivate_pages(map->pmap, bigobj, desired);
		VM_OBJECT_RUNLOCK(bigobj);
	}
	/*
	 * Next, hunt around for other pages to deactivate.  We actually
	 * do this search sort of wrong -- .text first is not the best idea.
	 */
	tmpe = map->header.next;
	while (tmpe != &map->header) {
		if (pmap_resident_count(vm_map_pmap(map)) <= desired)
			break;
		if ((tmpe->eflags & MAP_ENTRY_IS_SUB_MAP) == 0) {
			obj = tmpe->object.vm_object;
			if (obj != NULL) {
				VM_OBJECT_RLOCK(obj);
				vm_pageout_object_deactivate_pages(map->pmap, obj, desired);
				VM_OBJECT_RUNLOCK(obj);
			}
		}
		tmpe = tmpe->next;
	}

	/*
	 * Remove all mappings if a process is swapped out, this will free page
	 * table pages.
	 */
	if (desired == 0 && nothingwired) {
		pmap_remove(vm_map_pmap(map), vm_map_min(map),
		    vm_map_max(map));
	}

	vm_map_unlock(map);
}
#endif		/* !defined(NO_SWAPPING) */

/*
 * Attempt to acquire all of the necessary locks to launder a page and
 * then call through the clustering layer to PUTPAGES.  Wait a short
 * time for a vnode lock.
 *
 * Requires the page and object lock on entry, releases both before return.
 * Returns 0 on success and an errno otherwise.
 */
static int
vm_pageout_clean(vm_page_t m, int *numpagedout)
{
	struct vnode *vp;
	struct mount *mp;
	vm_object_t object;
	vm_pindex_t pindex;
	int error, lockmode;

	vm_page_assert_locked(m);
	object = m->object;
	VM_OBJECT_ASSERT_WLOCKED(object);
	error = 0;
	vp = NULL;
	mp = NULL;

	/*
	 * The object is already known NOT to be dead.   It
	 * is possible for the vget() to block the whole
	 * pageout daemon, but the new low-memory handling
	 * code should prevent it.
	 *
	 * We can't wait forever for the vnode lock, we might
	 * deadlock due to a vn_read() getting stuck in
	 * vm_wait while holding this vnode.  We skip the 
	 * vnode if we can't get it in a reasonable amount
	 * of time.
	 */
	if (object->type == OBJT_VNODE) {
		vm_page_unlock(m);
		vp = object->handle;
		if (vp->v_type == VREG &&
		    vn_start_write(vp, &mp, V_NOWAIT) != 0) {
			mp = NULL;
			error = EDEADLK;
			goto unlock_all;
		}
		KASSERT(mp != NULL,
		    ("vp %p with NULL v_mount", vp));
		vm_object_reference_locked(object);
		pindex = m->pindex;
		VM_OBJECT_WUNLOCK(object);
		lockmode = MNT_SHARED_WRITES(vp->v_mount) ?
		    LK_SHARED : LK_EXCLUSIVE;
		if (vget(vp, lockmode | LK_TIMELOCK, curthread)) {
			vp = NULL;
			error = EDEADLK;
			goto unlock_mp;
		}
		VM_OBJECT_WLOCK(object);
		vm_page_lock(m);
		/*
		 * While the object and page were unlocked, the page
		 * may have been:
		 * (1) moved to a different queue,
		 * (2) reallocated to a different object,
		 * (3) reallocated to a different offset, or
		 * (4) cleaned.
		 */
		if (m->queue != PQ_LAUNDRY || m->object != object ||
		    m->pindex != pindex || m->dirty == 0) {
			vm_page_unlock(m);
			error = ENXIO;
			goto unlock_all;
		}

		/*
		 * The page may have been busied or held while the object
		 * and page locks were released.
		 */
		if (vm_page_busied(m) || m->hold_count != 0) {
			vm_page_unlock(m);
			error = EBUSY;
			goto unlock_all;
		}
	}

	/*
	 * If a page is dirty, then it is either being washed
	 * (but not yet cleaned) or it is still in the
	 * laundry.  If it is still in the laundry, then we
	 * start the cleaning operation. 
	 */
	if ((*numpagedout = vm_pageout_cluster(m)) == 0)
		error = EIO;

unlock_all:
	VM_OBJECT_WUNLOCK(object);

unlock_mp:
	vm_page_lock_assert(m, MA_NOTOWNED);
	if (mp != NULL) {
		if (vp != NULL)
			vput(vp);
		vm_object_deallocate(object);
		vn_finished_write(mp);
	}

	return (error);
}

/*
 * XXX
 */
static void
vm_pageout_launder(struct vm_domain *vmd)
{
	vm_page_t m, next;
	struct vm_pagequeue *pq;
	vm_object_t object;
	int act_delta, error, launder, maxscan, numpagedout, pass;
	int vnodes_skipped;
	boolean_t pageout_ok, queues_locked;

	/*
	 * Compute the number of pages we want to move from the laundry queue to
	 * the inactive queue.  If there is no shortage of clean, inactive
	 * pages, we allow laundering to proceed at a trickle to ensure that
	 * dirty pages will eventually be reused.  Otherwise, the inactive queue
	 * target is scaled by the ratio of the sleep intervals of the laundry
	 * queue and inactive queue worker threads.
	 */
	launder = vm_cnt.v_inactive_target - vm_cnt.v_inactive_count +
	    vm_paging_target() + vm_pageout_deficit;
	if (launder < 0)
		launder = idle_launder_rate / VM_LAUNDER_INTERVAL;
	else
		launder /= VM_LAUNDER_RATE;

	vnodes_skipped = 0;

	/*
	 * Scan the laundry and stasis queues queue for pages eligible to be
	 * laundered.  We stop once the target number of dirty pages have been
	 * laundered, or once we've reached the ends of the queues.  A single
	 * iteration of this loop may cause more than one page to be laundered
	 * because of clustering.
	 *
	 * Pages in stasis are only examined when at least one swap device is
	 * available.  We don't attempt to scan the entire stasis queue; it is
	 * likely that many if not all of its pages cannot be paged out.  For
	 * now we only attempt to examine the target number of pages before
	 * falling back to the laundry queue.
	 *
	 * During the laundry queue scan, maxscan ensures that we don't
	 * re-examine requeued pages.  Any additional pages written as part of
	 * a cluster are subtracted from maxscan since they must be taken from
	 * the laundry queue.
	 */
	pass = 0;
scan:
	if (pass == 0) {
		if (vm_swapdev_cnt == 0) {
			pass++;
			goto scan;
		}
		pq = &vmd->vmd_pagequeues[PQ_STASIS];
		maxscan = launder;
	} else {
		pq = &vmd->vmd_pagequeues[PQ_LAUNDRY];
		maxscan = pq->pq_cnt;
	}

	vm_pagequeue_lock(pq);
	queues_locked = TRUE;
	for (m = TAILQ_FIRST(&pq->pq_pl);
	    m != NULL && maxscan-- > 0 && launder > 0;
	    m = next) {
		vm_pagequeue_assert_locked(pq);
		KASSERT(queues_locked, ("unlocked laundry queue"));
		KASSERT(m->queue == PQ_LAUNDRY,
		    ("page %p has an inconsistent queue", m));
		next = TAILQ_NEXT(m, plinks.q);
		if ((m->flags & PG_MARKER) != 0)
			continue;
		KASSERT((m->flags & PG_FICTITIOUS) == 0,
		    ("PG_FICTITIOUS page %p cannot be in laundry queue", m));
		KASSERT((m->oflags & VPO_UNMANAGED) == 0,
		    ("VPO_UNMANAGED page %p cannot be in laundry queue", m));
		if (!vm_pageout_page_lock(m, &next) || m->hold_count != 0) {
			vm_page_unlock(m);
			continue;
		}
		object = m->object;
		if ((!VM_OBJECT_TRYWLOCK(object) &&
		    (!vm_pageout_fallback_object_lock(m, &next) ||
		    m->hold_count != 0)) || vm_page_busied(m)) {
			VM_OBJECT_WUNLOCK(object);
			vm_page_unlock(m);
			continue;
		}

		/*
		 * We unlock the laundry queue, invalidating the
		 * 'next' pointer.  Use our marker to remember our
		 * place.
		 */
		TAILQ_INSERT_AFTER(&pq->pq_pl, m, &vmd->vmd_laundry_marker,
		    plinks.q);
		vm_pagequeue_unlock(pq);
		queues_locked = FALSE;

		/*
		 * Invalid pages can be easily freed.  They cannot be
		 * mapped; vm_page_free() asserts this.
		 */
		if (m->valid == 0)
			goto free_page;

		/*
		 * If the page has been referenced and the object is not dead,
		 * reactivate or requeue the page depending on whether the
		 * object is mapped.
		 */
		if ((m->aflags & PGA_REFERENCED) != 0) {
			vm_page_aflag_clear(m, PGA_REFERENCED);
			act_delta = 1;
		} else
			act_delta = 0;
		if (object->ref_count != 0)
			act_delta += pmap_ts_referenced(m);
		else {
			KASSERT(!pmap_page_is_mapped(m),
			    ("page %p is mapped", m));
		}
		if (act_delta != 0) {
			if (object->ref_count != 0) {
				vm_page_activate(m);

				/*
				 * Increase the activation count if the page
				 * was referenced while in the laundry queue.
				 * This makes it less likely that the page will
				 * be returned prematurely to the inactive
				 * queue.
 				 */
				m->act_count += act_delta + ACT_ADVANCE;
				goto drop_page;
			} else if ((object->flags & OBJ_DEAD) == 0)
				goto requeue_page;
		}

		/*
		 * If the page appears to be clean at the machine-independent
		 * layer, then remove all of its mappings from the pmap in
		 * anticipation of placing it onto the cache queue.  If,
		 * however, any of the page's mappings allow write access,
		 * then the page may still be modified until the last of those
		 * mappings are removed.
		 */
		if (object->ref_count != 0) {
			vm_page_test_dirty(m);
			if (m->dirty == 0)
				pmap_remove_all(m);
		}

		/*
		 * Clean pages are freed, and dirty pages are paged out unless
		 * they belong to a dead object.  Requeueing dirty pages from
		 * dead objects is pointless, as they are being paged out and
		 * freed by the thread that destroyed the object.
		 */
		if (m->dirty == 0) {
free_page:
			vm_page_free(m);
			PCPU_INC(cnt.v_dfree);
		} else if ((object->flags & OBJ_DEAD) == 0) {
			if (object->type != OBJT_SWAP &&
			    object->type != OBJT_DEFAULT)
				pageout_ok = TRUE;
			else if (disable_swap_pageouts)
				pageout_ok = FALSE;
			else
				pageout_ok = TRUE;
			if (!pageout_ok) {
requeue_page:
				vm_pagequeue_lock(pq);
				queues_locked = TRUE;
				vm_page_requeue_locked(m);
				goto drop_page;
			}
			error = vm_pageout_clean(m, &numpagedout);
			if (error == 0) {
				launder -= numpagedout;
				maxscan -= numpagedout - 1;
			} else if (error == EDEADLK) {
				pageout_lock_miss++;
				vnodes_skipped++;
			}
			goto relock_queues;
		}
drop_page:
		vm_page_unlock(m);
		VM_OBJECT_WUNLOCK(object);
relock_queues:
		if (!queues_locked) {
			vm_pagequeue_lock(pq);
			queues_locked = TRUE;
		}
		next = TAILQ_NEXT(&vmd->vmd_laundry_marker, plinks.q);
		TAILQ_REMOVE(&pq->pq_pl, &vmd->vmd_laundry_marker, plinks.q);
	}
	vm_pagequeue_unlock(pq);

	/*
	 * If we didn't meet our target with the stasis queue, try again using
	 * the laundry queue.
	 */
	if (pass == 0 && launder > 0) {
		pass++;
		goto scan;
	}

	/*
	 * Wakeup the sync daemon if we skipped a vnode in a writeable object
	 * and we didn't launder enough pages.
	 */
	if (vnodes_skipped > 0 && launder > 0)
		(void)speedup_syncer();
}

/*
 * XXX
 */
static void
vm_pageout_swapon(void *arg, struct swdevt *sp __unused)
{
	struct pglist pl;
	struct vm_domain *vmd;
	struct vm_pagequeue *pq;
	vm_page_t m, next;

	vmd = arg;
	TAILQ_INIT(&pl);
	if (vm_swapdev_cnt++ == 0) {
		pq = &vmd->vmd_pagequeues[PQ_STASIS];
		/*
		 * We now have a swap device, so migrate pages back to the
		 * laundry queue.  Locking rules make this somewhat awkward, but
		 * this is a rare operation.
		 */
		vm_pagequeue_lock(pq);
		while ((m = TAILQ_FIRST(&pq->pq_pl)) != NULL) {
			if (!vm_pageout_page_lock(m, &next)) {
				vm_page_unlock(m);
				continue;
			}
			vm_page_dequeue_locked(m);
			vm_pagequeue_unlock(pq);
			vm_page_launder(m);
			vm_page_unlock(m);
			vm_pagequeue_lock(pq);
		}
		vm_pagequeue_unlock(pq);
	}
}

/*
 * XXX
 */
static void
vm_pageout_swapoff(void *arg __unused, struct swdevt *sp __unused)
{

	vm_swapdev_cnt--;
}

/*
 * XXX
 */
static void
vm_pageout_laundry_worker(void *arg)
{
	struct vm_domain *domain;
	int domidx;

	domidx = (uintptr_t)arg;
	domain = &vm_dom[domidx];
	KASSERT(domain->vmd_segs != 0, ("domain without segments"));
	vm_pageout_init_marker(&domain->vmd_laundry_marker, PQ_LAUNDRY);

	/*
	 * Calls to these handlers are serialized by the swapconf lock.
	 */
	(void)EVENTHANDLER_REGISTER(swapon, vm_pageout_swapon, domain,
	    EVENTHANDLER_PRI_ANY);
	(void)EVENTHANDLER_REGISTER(swapoff, vm_pageout_swapoff, domain,
	    EVENTHANDLER_PRI_ANY);

	/*
	 * The pageout laundry worker is never done, so loop forever.
	 */
	for (;;) {
		tsleep(&vm_cnt.v_laundry_count, PVM, "laundr",
		    hz / VM_LAUNDER_INTERVAL);
		vm_pageout_launder(domain);
	}
}

/*
 *	vm_pageout_scan does the dirty work for the pageout daemon.
 *
 *	pass 0 - Update active LRU/deactivate pages
 *	pass 1 - Move inactive to cache or free
 */
static void
vm_pageout_scan(struct vm_domain *vmd, int pass)
{
	vm_page_t m, next;
	struct vm_pagequeue *pq;
	vm_object_t object;
	long min_scan;
	int act_delta, addl_page_shortage, deficit, maxscan;
	int page_shortage, scan_tick, scanned, starting_page_shortage;
	boolean_t queues_locked;

	/*
	 * If we need to reclaim memory ask kernel caches to return
	 * some.  We rate limit to avoid thrashing.
	 */
	if (vmd == &vm_dom[0] && pass > 0 &&
	    (time_uptime - lowmem_uptime) >= lowmem_period) {
		/*
		 * Decrease registered cache sizes.
		 */
		SDT_PROBE0(vm, , , vm__lowmem_scan);
		EVENTHANDLER_INVOKE(vm_lowmem, 0);
		/*
		 * We do this explicitly after the caches have been
		 * drained above.
		 */
		uma_reclaim();
		lowmem_uptime = time_uptime;
	}

	/*
	 * The addl_page_shortage is the number of temporarily
	 * stuck pages in the inactive queue.  In other words, the
	 * number of pages from the inactive count that should be
	 * discounted in setting the target for the active queue scan.
	 */
	addl_page_shortage = 0;

	/*
	 * Calculate the number of pages we want to either free or move
	 * to the cache.
	 */
	if (pass > 0) {
		deficit = atomic_readandclear_int(&vm_pageout_deficit);
		page_shortage = vm_paging_target() + deficit;
	} else
		page_shortage = deficit = 0;
	starting_page_shortage = page_shortage;

	/*
	 * Start scanning the inactive queue for pages we can move to the
	 * cache or free.  The scan will stop when the target is reached or
	 * we have scanned the entire inactive queue.  Note that m->act_count
	 * is not used to form decisions for the inactive queue, only for the
	 * active queue.
	 */
	pq = &vmd->vmd_pagequeues[PQ_INACTIVE];
	maxscan = pq->pq_cnt;
	vm_pagequeue_lock(pq);
	queues_locked = TRUE;
	for (m = TAILQ_FIRST(&pq->pq_pl);
	     m != NULL && maxscan-- > 0 && page_shortage > 0;
	     m = next) {
		vm_pagequeue_assert_locked(pq);
		KASSERT(queues_locked, ("unlocked queues"));
		KASSERT(m->queue == PQ_INACTIVE, ("Inactive queue %p", m));

		PCPU_INC(cnt.v_pdpages);
		next = TAILQ_NEXT(m, plinks.q);

		/*
		 * skip marker pages
		 */
		if (m->flags & PG_MARKER)
			continue;

		KASSERT((m->flags & PG_FICTITIOUS) == 0,
		    ("Fictitious page %p cannot be in inactive queue", m));
		KASSERT((m->oflags & VPO_UNMANAGED) == 0,
		    ("Unmanaged page %p cannot be in inactive queue", m));

		/*
		 * The page or object lock acquisitions fail if the
		 * page was removed from the queue or moved to a
		 * different position within the queue.  In either
		 * case, addl_page_shortage should not be incremented.
		 */
		if (!vm_pageout_page_lock(m, &next))
			goto unlock_page;
		else if (m->hold_count != 0) {
			/*
			 * Held pages are essentially stuck in the
			 * queue.  So, they ought to be discounted
			 * from the inactive count.  See the
			 * calculation of the page_shortage for the
			 * loop over the active queue below.
			 */
			addl_page_shortage++;
			goto unlock_page;
		}
		object = m->object;
		if (!VM_OBJECT_TRYWLOCK(object)) {
			if (!vm_pageout_fallback_object_lock(m, &next))
				goto unlock_object;
			else if (m->hold_count != 0) {
				addl_page_shortage++;
				goto unlock_object;
			}
		}
		if (vm_page_busied(m)) {
			/*
			 * Don't mess with busy pages.  Leave them at
			 * the front of the queue.  Most likely, they
			 * are being paged out and will leave the
			 * queue shortly after the scan finishes.  So,
			 * they ought to be discounted from the
			 * inactive count.
			 */
			addl_page_shortage++;
unlock_object:
			VM_OBJECT_WUNLOCK(object);
unlock_page:
			vm_page_unlock(m);
			continue;
		}
		KASSERT(m->hold_count == 0, ("Held page %p", m));

		/*
		 * We unlock the inactive page queue, invalidating the
		 * 'next' pointer.  Use our marker to remember our
		 * place.
		 */
		TAILQ_INSERT_AFTER(&pq->pq_pl, m, &vmd->vmd_marker, plinks.q);
		vm_page_dequeue_locked(m);
		vm_pagequeue_unlock(pq);
		queues_locked = FALSE;

		/*
		 * Invalid pages can be easily freed. They cannot be
		 * mapped, vm_page_free() asserts this.
		 */
		if (m->valid == 0)
			goto free_page;

		/*
		 * If the page has been referenced and the object is not dead,
		 * reactivate or requeue the page depending on whether the
		 * object is mapped.
		 */
		if ((m->aflags & PGA_REFERENCED) != 0) {
			vm_page_aflag_clear(m, PGA_REFERENCED);
			act_delta = 1;
		} else
			act_delta = 0;
		if (object->ref_count != 0) {
			act_delta += pmap_ts_referenced(m);
		} else {
			KASSERT(!pmap_page_is_mapped(m),
			    ("vm_pageout_scan: page %p is mapped", m));
		}
		if (act_delta != 0) {
			if (object->ref_count != 0) {
				vm_page_activate(m);

				/*
				 * Increase the activation count if the page
				 * was referenced while in the inactive queue.
				 * This makes it less likely that the page will
				 * be returned prematurely to the inactive
				 * queue.
 				 */
				m->act_count += act_delta + ACT_ADVANCE;
				goto drop_page;
			} else if ((object->flags & OBJ_DEAD) == 0) {
				vm_pagequeue_lock(pq);
				queues_locked = TRUE;
				m->queue = PQ_INACTIVE;
				TAILQ_INSERT_TAIL(&pq->pq_pl, m, plinks.q);
				vm_pagequeue_cnt_inc(pq);
				goto drop_page;
			}
		}

		/*
		 * If the page appears to be clean at the machine-independent
		 * layer, then remove all of its mappings from the pmap in
		 * anticipation of placing it onto the cache queue.  If,
		 * however, any of the page's mappings allow write access,
		 * then the page may still be modified until the last of those
		 * mappings are removed.
		 */
		if (object->ref_count != 0) {
			vm_page_test_dirty(m);
			if (m->dirty == 0)
				pmap_remove_all(m);
		}

		/*
		 * Clean pages can be freed, but dirty pages must be sent back
		 * to the laundry, unless they belong to a dead object.
		 * Requeueing dirty pages from dead objects is pointless, as
		 * they are being paged out and freed by the thread that
		 * destroyed the object.
		 */
		if (m->dirty == 0) {
free_page:
			vm_page_free(m);
			PCPU_INC(cnt.v_dfree);
			--page_shortage;
		} else if ((object->flags & OBJ_DEAD) == 0)
			vm_page_launder(m);
drop_page:
		vm_page_unlock(m);
		VM_OBJECT_WUNLOCK(object);
		if (!queues_locked) {
			vm_pagequeue_lock(pq);
			queues_locked = TRUE;
		}
		next = TAILQ_NEXT(&vmd->vmd_marker, plinks.q);
		TAILQ_REMOVE(&pq->pq_pl, &vmd->vmd_marker, plinks.q);
	}
	vm_pagequeue_unlock(pq);

	/*
	 * Wakeup the laundry thread(s) if we didn't free the targeted number
	 * of pages.
	 */
	if (page_shortage > 0)
		wakeup(&vm_cnt.v_laundry_count);

#if !defined(NO_SWAPPING)
	/*
	 * Wakeup the swapout daemon if we didn't cache or free the targeted
	 * number of pages. 
	 */
	if (vm_swap_enabled && page_shortage > 0)
		vm_req_vmdaemon(VM_SWAP_NORMAL);
#endif

	/*
	 * If the inactive queue scan fails repeatedly to meet its
	 * target, kill the largest process.
	 */
	vm_pageout_mightbe_oom(vmd, page_shortage, starting_page_shortage);

	/*
	 * Compute the number of pages we want to try to move from the
	 * active queue to either the inactive or laundry queue.
	 *
	 * When scanning active pages, we make clean pages count more heavily
	 * towards the page shortage than dirty pages.  This is because dirty
	 * pages must be laundered before they can be reused and thus have less
	 * utility when attempting to quickly alleviate a shortage.  However,
	 * this weighting also causes the scan to deactivate dirty pages more
	 * more aggressively, improving the effectiveness of clustering and
	 * ensuring that they can eventually be reused.
	 */
	page_shortage = vm_cnt.v_inactive_target - (vm_cnt.v_inactive_count +
	    vm_cnt.v_laundry_count / act_scan_laundry_weight) +
	    vm_paging_target() + deficit + addl_page_shortage;
	page_shortage *= act_scan_laundry_weight;

	pq = &vmd->vmd_pagequeues[PQ_ACTIVE];
	vm_pagequeue_lock(pq);
	maxscan = pq->pq_cnt;

	/*
	 * If we're just idle polling attempt to visit every
	 * active page within 'update_period' seconds.
	 */
	scan_tick = ticks;
	if (vm_pageout_update_period != 0) {
		min_scan = pq->pq_cnt;
		min_scan *= scan_tick - vmd->vmd_last_active_scan;
		min_scan /= hz * vm_pageout_update_period;
	} else
		min_scan = 0;
	if (min_scan > 0 || (page_shortage > 0 && maxscan > 0))
		vmd->vmd_last_active_scan = scan_tick;

	/*
	 * Scan the active queue for pages that can be deactivated.  Update
	 * the per-page activity counter and use it to identify deactivation
	 * candidates.
	 */
	for (m = TAILQ_FIRST(&pq->pq_pl), scanned = 0; m != NULL && (scanned <
	    min_scan || (page_shortage > 0 && scanned < maxscan)); m = next,
	    scanned++) {

		KASSERT(m->queue == PQ_ACTIVE,
		    ("vm_pageout_scan: page %p isn't active", m));

		next = TAILQ_NEXT(m, plinks.q);
		if ((m->flags & PG_MARKER) != 0)
			continue;
		KASSERT((m->flags & PG_FICTITIOUS) == 0,
		    ("Fictitious page %p cannot be in active queue", m));
		KASSERT((m->oflags & VPO_UNMANAGED) == 0,
		    ("Unmanaged page %p cannot be in active queue", m));
		if (!vm_pageout_page_lock(m, &next)) {
			vm_page_unlock(m);
			continue;
		}

		/*
		 * The count for pagedaemon pages is done after checking the
		 * page for eligibility...
		 */
		PCPU_INC(cnt.v_pdpages);

		/*
		 * Check to see "how much" the page has been used.
		 */
		if ((m->aflags & PGA_REFERENCED) != 0) {
			vm_page_aflag_clear(m, PGA_REFERENCED);
			act_delta = 1;
		} else
			act_delta = 0;

		/*
		 * Unlocked object ref count check.  Two races are possible.
		 * 1) The ref was transitioning to zero and we saw non-zero,
		 *    the pmap bits will be checked unnecessarily.
		 * 2) The ref was transitioning to one and we saw zero. 
		 *    The page lock prevents a new reference to this page so
		 *    we need not check the reference bits.
		 */
		if (m->object->ref_count != 0)
			act_delta += pmap_ts_referenced(m);

		/*
		 * Advance or decay the act_count based on recent usage.
		 */
		if (act_delta != 0) {
			m->act_count += ACT_ADVANCE + act_delta;
			if (m->act_count > ACT_MAX)
				m->act_count = ACT_MAX;
		} else
			m->act_count -= min(m->act_count, ACT_DECLINE);

		/*
		 * Move this page to the tail of the active, inactive or laundry
		 * queue depending on usage.
		 */
		if (m->act_count == 0) {
			/* Dequeue to avoid later lock recursion. */
			vm_page_dequeue_locked(m);
#if 0
			if (m->object->ref_count != 0)
				vm_page_test_dirty(m);
#endif
			if (m->dirty == 0) {
				vm_page_deactivate(m);
				page_shortage -= act_scan_laundry_weight;
			} else {
				vm_page_launder(m);
				page_shortage--;
			}
		} else
			vm_page_requeue_locked(m);
		vm_page_unlock(m);
	}
	vm_pagequeue_unlock(pq);
#if !defined(NO_SWAPPING)
	/*
	 * Idle process swapout -- run once per second.
	 */
	if (vm_swap_idle_enabled) {
		static long lsec;
		if (time_second != lsec) {
			vm_req_vmdaemon(VM_SWAP_IDLE);
			lsec = time_second;
		}
	}
#endif
}

static int vm_pageout_oom_vote;

/*
 * The pagedaemon threads randlomly select one to perform the
 * OOM.  Trying to kill processes before all pagedaemons
 * failed to reach free target is premature.
 */
static void
vm_pageout_mightbe_oom(struct vm_domain *vmd, int page_shortage,
    int starting_page_shortage)
{
	int old_vote;

	if (starting_page_shortage <= 0 || starting_page_shortage !=
	    page_shortage)
		vmd->vmd_oom_seq = 0;
	else
		vmd->vmd_oom_seq++;
	if (vmd->vmd_oom_seq < vm_pageout_oom_seq) {
		if (vmd->vmd_oom) {
			vmd->vmd_oom = FALSE;
			atomic_subtract_int(&vm_pageout_oom_vote, 1);
		}
		return;
	}

	/*
	 * Do not follow the call sequence until OOM condition is
	 * cleared.
	 */
	vmd->vmd_oom_seq = 0;

	if (vmd->vmd_oom)
		return;

	vmd->vmd_oom = TRUE;
	old_vote = atomic_fetchadd_int(&vm_pageout_oom_vote, 1);
	if (old_vote != vm_ndomains - 1)
		return;

	/*
	 * The current pagedaemon thread is the last in the quorum to
	 * start OOM.  Initiate the selection and signaling of the
	 * victim.
	 */
	vm_pageout_oom(VM_OOM_MEM);

	/*
	 * After one round of OOM terror, recall our vote.  On the
	 * next pass, current pagedaemon would vote again if the low
	 * memory condition is still there, due to vmd_oom being
	 * false.
	 */
	vmd->vmd_oom = FALSE;
	atomic_subtract_int(&vm_pageout_oom_vote, 1);
}

/*
 * The OOM killer is the page daemon's action of last resort when
 * memory allocation requests have been stalled for a prolonged period
 * of time because it cannot reclaim memory.  This function computes
 * the approximate number of physical pages that could be reclaimed if
 * the specified address space is destroyed.
 *
 * Private, anonymous memory owned by the address space is the
 * principal resource that we expect to recover after an OOM kill.
 * Since the physical pages mapped by the address space's COW entries
 * are typically shared pages, they are unlikely to be released and so
 * they are not counted.
 *
 * To get to the point where the page daemon runs the OOM killer, its
 * efforts to write-back vnode-backed pages may have stalled.  This
 * could be caused by a memory allocation deadlock in the write path
 * that might be resolved by an OOM kill.  Therefore, physical pages
 * belonging to vnode-backed objects are counted, because they might
 * be freed without being written out first if the address space holds
 * the last reference to an unlinked vnode.
 *
 * Similarly, physical pages belonging to OBJT_PHYS objects are
 * counted because the address space might hold the last reference to
 * the object.
 */
static long
vm_pageout_oom_pagecount(struct vmspace *vmspace)
{
	vm_map_t map;
	vm_map_entry_t entry;
	vm_object_t obj;
	long res;

	map = &vmspace->vm_map;
	KASSERT(!map->system_map, ("system map"));
	sx_assert(&map->lock, SA_LOCKED);
	res = 0;
	for (entry = map->header.next; entry != &map->header;
	    entry = entry->next) {
		if ((entry->eflags & MAP_ENTRY_IS_SUB_MAP) != 0)
			continue;
		obj = entry->object.vm_object;
		if (obj == NULL)
			continue;
		if ((entry->eflags & MAP_ENTRY_NEEDS_COPY) != 0 &&
		    obj->ref_count != 1)
			continue;
		switch (obj->type) {
		case OBJT_DEFAULT:
		case OBJT_SWAP:
		case OBJT_PHYS:
		case OBJT_VNODE:
			res += obj->resident_page_count;
			break;
		}
	}
	return (res);
}

void
vm_pageout_oom(int shortage)
{
	struct proc *p, *bigproc;
	vm_offset_t size, bigsize;
	struct thread *td;
	struct vmspace *vm;

	/*
	 * We keep the process bigproc locked once we find it to keep anyone
	 * from messing with it; however, there is a possibility of
	 * deadlock if process B is bigproc and one of it's child processes
	 * attempts to propagate a signal to B while we are waiting for A's
	 * lock while walking this list.  To avoid this, we don't block on
	 * the process lock but just skip a process if it is already locked.
	 */
	bigproc = NULL;
	bigsize = 0;
	sx_slock(&allproc_lock);
	FOREACH_PROC_IN_SYSTEM(p) {
		int breakout;

		PROC_LOCK(p);

		/*
		 * If this is a system, protected or killed process, skip it.
		 */
		if (p->p_state != PRS_NORMAL || (p->p_flag & (P_INEXEC |
		    P_PROTECTED | P_SYSTEM | P_WEXIT)) != 0 ||
		    p->p_pid == 1 || P_KILLED(p) ||
		    (p->p_pid < 48 && swap_pager_avail != 0)) {
			PROC_UNLOCK(p);
			continue;
		}
		/*
		 * If the process is in a non-running type state,
		 * don't touch it.  Check all the threads individually.
		 */
		breakout = 0;
		FOREACH_THREAD_IN_PROC(p, td) {
			thread_lock(td);
			if (!TD_ON_RUNQ(td) &&
			    !TD_IS_RUNNING(td) &&
			    !TD_IS_SLEEPING(td) &&
			    !TD_IS_SUSPENDED(td) &&
			    !TD_IS_SWAPPED(td)) {
				thread_unlock(td);
				breakout = 1;
				break;
			}
			thread_unlock(td);
		}
		if (breakout) {
			PROC_UNLOCK(p);
			continue;
		}
		/*
		 * get the process size
		 */
		vm = vmspace_acquire_ref(p);
		if (vm == NULL) {
			PROC_UNLOCK(p);
			continue;
		}
		_PHOLD(p);
		if (!vm_map_trylock_read(&vm->vm_map)) {
			_PRELE(p);
			PROC_UNLOCK(p);
			vmspace_free(vm);
			continue;
		}
		PROC_UNLOCK(p);
		size = vmspace_swap_count(vm);
		if (shortage == VM_OOM_MEM)
			size += vm_pageout_oom_pagecount(vm);
		vm_map_unlock_read(&vm->vm_map);
		vmspace_free(vm);

		/*
		 * If this process is bigger than the biggest one,
		 * remember it.
		 */
		if (size > bigsize) {
			if (bigproc != NULL)
				PRELE(bigproc);
			bigproc = p;
			bigsize = size;
		} else {
			PRELE(p);
		}
	}
	sx_sunlock(&allproc_lock);
	if (bigproc != NULL) {
		if (vm_panic_on_oom != 0)
			panic("out of swap space");
		PROC_LOCK(bigproc);
		killproc(bigproc, "out of swap space");
		sched_nice(bigproc, PRIO_MIN);
		_PRELE(bigproc);
		PROC_UNLOCK(bigproc);
		wakeup(&vm_cnt.v_free_count);
	}
}

static void
vm_pageout_worker(void *arg)
{
	struct vm_domain *domain;
	int domidx;

	domidx = (uintptr_t)arg;
	domain = &vm_dom[domidx];

	/*
	 * XXXKIB It could be useful to bind pageout daemon threads to
	 * the cores belonging to the domain, from which vm_page_array
	 * is allocated.
	 */

	KASSERT(domain->vmd_segs != 0, ("domain without segments"));
	domain->vmd_last_active_scan = ticks;
	vm_pageout_init_marker(&domain->vmd_marker, PQ_INACTIVE);
	vm_pageout_init_marker(&domain->vmd_inacthead, PQ_INACTIVE);
	TAILQ_INSERT_HEAD(&domain->vmd_pagequeues[PQ_INACTIVE].pq_pl,
	    &domain->vmd_inacthead, plinks.q);

	/*
	 * The pageout daemon worker is never done, so loop forever.
	 */
	while (TRUE) {
		/*
		 * If we have enough free memory, wakeup waiters.  Do
		 * not clear vm_pages_needed until we reach our target,
		 * otherwise we may be woken up over and over again and
		 * waste a lot of cpu.
		 */
		mtx_lock(&vm_page_queue_free_mtx);
		if (vm_pages_needed && !vm_page_count_min()) {
			if (!vm_paging_needed())
				vm_pages_needed = 0;
			wakeup(&vm_cnt.v_free_count);
		}
		if (vm_pages_needed) {
			/*
			 * We're still not done.  Either vm_pages_needed was
			 * set by another thread during the previous scan
			 * (typically, this happens during a level 0 scan) or
			 * vm_pages_needed was already set and the scan failed
			 * to free enough pages.  If we've only performed a
			 * level 0 scan, then upgrade the level and scan again
			 * now.  Otherwise, sleep a bit and try again later.
			 * While sleeping, vm_pages_needed can be cleared.
			 */
			if (domain->vmd_pass > 0)
				msleep(&vm_pages_needed,
				    &vm_page_queue_free_mtx, PVM, "psleep",
				    hz / VM_INACT_SCAN_INTERVAL);
		} else {
			/*
			 * Good enough, sleep until required to refresh
			 * stats.
			 */
			msleep(&vm_pages_needed, &vm_page_queue_free_mtx,
			    PVM, "psleep", hz);
		}
		if (vm_pages_needed) {
			vm_cnt.v_pdwakeups++;
			domain->vmd_pass++;
		} else
			domain->vmd_pass = 0;
		mtx_unlock(&vm_page_queue_free_mtx);
		vm_pageout_scan(domain, domain->vmd_pass);
	}
}

/*
 *	vm_pageout_init initialises basic pageout daemon settings.
 */
static void
vm_pageout_init(void)
{
	/*
	 * Initialize some paging parameters.
	 */
	vm_cnt.v_interrupt_free_min = 2;
	if (vm_cnt.v_page_count < 2000)
		vm_pageout_page_count = 8;

	/*
	 * v_free_reserved needs to include enough for the largest
	 * swap pager structures plus enough for any pv_entry structs
	 * when paging. 
	 */
	if (vm_cnt.v_page_count > 1024)
		vm_cnt.v_free_min = 4 + (vm_cnt.v_page_count - 1024) / 200;
	else
		vm_cnt.v_free_min = 4;
	vm_cnt.v_pageout_free_min = (2*MAXBSIZE)/PAGE_SIZE +
	    vm_cnt.v_interrupt_free_min;
	vm_cnt.v_free_reserved = vm_pageout_page_count +
	    vm_cnt.v_pageout_free_min + (vm_cnt.v_page_count / 768);
	vm_cnt.v_free_severe = vm_cnt.v_free_min / 2;
	vm_cnt.v_free_target = 4 * vm_cnt.v_free_min + vm_cnt.v_free_reserved;
	vm_cnt.v_free_min += vm_cnt.v_free_reserved;
	vm_cnt.v_free_severe += vm_cnt.v_free_reserved;
	vm_cnt.v_inactive_target = (3 * vm_cnt.v_free_target) / 2;
	if (vm_cnt.v_inactive_target > vm_cnt.v_free_count / 3)
		vm_cnt.v_inactive_target = vm_cnt.v_free_count / 3;

	/*
	 * Set the default wakeup threshold to be 10% above the minimum
	 * page limit.  This keeps the steady state out of shortfall.
	 */
	vm_pageout_wakeup_thresh = (vm_cnt.v_free_min / 10) * 11;

	/*
	 * Set interval in seconds for active scan.  We want to visit each
	 * page at least once every ten minutes.  This is to prevent worst
	 * case paging behaviors with stale active LRU.
	 */
	if (vm_pageout_update_period == 0)
		vm_pageout_update_period = 600;

	/* XXX does not really belong here */
	if (vm_page_max_wired == 0)
		vm_page_max_wired = vm_cnt.v_free_count / 3;
}

/*
 *     vm_pageout is the high level pageout daemon.
 */
static void
vm_pageout(void)
{
	int error;
#if MAXMEMDOM > 1
	int i;
#endif

	swap_pager_swap_init();
	error = kthread_add(vm_pageout_laundry_worker, NULL, curproc, NULL,
	    0, 0, "laundry: dom0");
	if (error != 0)
		panic("starting laundry for domain 0, error %d", error);
#if MAXMEMDOM > 1
	for (i = 1; i < vm_ndomains; i++) {
		error = kthread_add(vm_pageout_worker, (void *)(uintptr_t)i,
		    curproc, NULL, 0, 0, "dom%d", i);
		if (error != 0) {
			panic("starting pageout for domain %d, error %d\n",
			    i, error);
		}
	}
#endif
	error = kthread_add(uma_reclaim_worker, NULL, curproc, NULL,
	    0, 0, "uma");
	if (error != 0)
		panic("starting uma_reclaim helper, error %d\n", error);
	vm_pageout_worker((void *)(uintptr_t)0);
}

/*
 * Unless the free page queue lock is held by the caller, this function
 * should be regarded as advisory.  Specifically, the caller should
 * not msleep() on &vm_cnt.v_free_count following this function unless
 * the free page queue lock is held until the msleep() is performed.
 */
void
pagedaemon_wakeup(void)
{

	if (!vm_pages_needed && curthread->td_proc != pageproc) {
		vm_pages_needed = 1;
		wakeup(&vm_pages_needed);
	}
}

#if !defined(NO_SWAPPING)
static void
vm_req_vmdaemon(int req)
{
	static int lastrun = 0;

	mtx_lock(&vm_daemon_mtx);
	vm_pageout_req_swapout |= req;
	if ((ticks > (lastrun + hz)) || (ticks < lastrun)) {
		wakeup(&vm_daemon_needed);
		lastrun = ticks;
	}
	mtx_unlock(&vm_daemon_mtx);
}

static void
vm_daemon(void)
{
	struct rlimit rsslim;
	struct proc *p;
	struct thread *td;
	struct vmspace *vm;
	int breakout, swapout_flags, tryagain, attempts;
#ifdef RACCT
	uint64_t rsize, ravailable;
#endif

	while (TRUE) {
		mtx_lock(&vm_daemon_mtx);
		msleep(&vm_daemon_needed, &vm_daemon_mtx, PPAUSE, "psleep",
#ifdef RACCT
		    racct_enable ? hz : 0
#else
		    0
#endif
		);
		swapout_flags = vm_pageout_req_swapout;
		vm_pageout_req_swapout = 0;
		mtx_unlock(&vm_daemon_mtx);
		if (swapout_flags)
			swapout_procs(swapout_flags);

		/*
		 * scan the processes for exceeding their rlimits or if
		 * process is swapped out -- deactivate pages
		 */
		tryagain = 0;
		attempts = 0;
again:
		attempts++;
		sx_slock(&allproc_lock);
		FOREACH_PROC_IN_SYSTEM(p) {
			vm_pindex_t limit, size;

			/*
			 * if this is a system process or if we have already
			 * looked at this process, skip it.
			 */
			PROC_LOCK(p);
			if (p->p_state != PRS_NORMAL ||
			    p->p_flag & (P_INEXEC | P_SYSTEM | P_WEXIT)) {
				PROC_UNLOCK(p);
				continue;
			}
			/*
			 * if the process is in a non-running type state,
			 * don't touch it.
			 */
			breakout = 0;
			FOREACH_THREAD_IN_PROC(p, td) {
				thread_lock(td);
				if (!TD_ON_RUNQ(td) &&
				    !TD_IS_RUNNING(td) &&
				    !TD_IS_SLEEPING(td) &&
				    !TD_IS_SUSPENDED(td)) {
					thread_unlock(td);
					breakout = 1;
					break;
				}
				thread_unlock(td);
			}
			if (breakout) {
				PROC_UNLOCK(p);
				continue;
			}
			/*
			 * get a limit
			 */
			lim_rlimit_proc(p, RLIMIT_RSS, &rsslim);
			limit = OFF_TO_IDX(
			    qmin(rsslim.rlim_cur, rsslim.rlim_max));

			/*
			 * let processes that are swapped out really be
			 * swapped out set the limit to nothing (will force a
			 * swap-out.)
			 */
			if ((p->p_flag & P_INMEM) == 0)
				limit = 0;	/* XXX */
			vm = vmspace_acquire_ref(p);
			PROC_UNLOCK(p);
			if (vm == NULL)
				continue;

			size = vmspace_resident_count(vm);
			if (size >= limit) {
				vm_pageout_map_deactivate_pages(
				    &vm->vm_map, limit);
			}
#ifdef RACCT
			if (racct_enable) {
				rsize = IDX_TO_OFF(size);
				PROC_LOCK(p);
				racct_set(p, RACCT_RSS, rsize);
				ravailable = racct_get_available(p, RACCT_RSS);
				PROC_UNLOCK(p);
				if (rsize > ravailable) {
					/*
					 * Don't be overly aggressive; this
					 * might be an innocent process,
					 * and the limit could've been exceeded
					 * by some memory hog.  Don't try
					 * to deactivate more than 1/4th
					 * of process' resident set size.
					 */
					if (attempts <= 8) {
						if (ravailable < rsize -
						    (rsize / 4)) {
							ravailable = rsize -
							    (rsize / 4);
						}
					}
					vm_pageout_map_deactivate_pages(
					    &vm->vm_map,
					    OFF_TO_IDX(ravailable));
					/* Update RSS usage after paging out. */
					size = vmspace_resident_count(vm);
					rsize = IDX_TO_OFF(size);
					PROC_LOCK(p);
					racct_set(p, RACCT_RSS, rsize);
					PROC_UNLOCK(p);
					if (rsize > ravailable)
						tryagain = 1;
				}
			}
#endif
			vmspace_free(vm);
		}
		sx_sunlock(&allproc_lock);
		if (tryagain != 0 && attempts <= 10)
			goto again;
	}
}
#endif			/* !defined(NO_SWAPPING) */
