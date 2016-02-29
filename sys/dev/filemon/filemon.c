/*-
 * Copyright (c) 2011, David E. O'Brien.
 * Copyright (c) 2009-2011, Juniper Networks, Inc.
 * Copyright (c) 2015, EMC Corp.
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY JUNIPER NETWORKS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL JUNIPER NETWORKS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/file.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/capsicum.h>
#include <sys/condvar.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/ioccom.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/sx.h>
#include <sys/syscall.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/uio.h>

#include "filemon.h"

#if defined(COMPAT_IA32) || defined(COMPAT_FREEBSD32) || defined(COMPAT_ARCH32)
#include <compat/freebsd32/freebsd32_syscall.h>
#include <compat/freebsd32/freebsd32_proto.h>

extern struct sysentvec ia32_freebsd_sysvec;
#endif

extern struct sysentvec elf32_freebsd_sysvec;
extern struct sysentvec elf64_freebsd_sysvec;

static d_close_t	filemon_close;
static d_ioctl_t	filemon_ioctl;
static d_open_t		filemon_open;
static int		filemon_unload(void);
static void		filemon_load(void *);

static struct cdevsw filemon_cdevsw = {
	.d_version	= D_VERSION,
	.d_close	= filemon_close,
	.d_ioctl	= filemon_ioctl,
	.d_open		= filemon_open,
	.d_name		= "filemon",
};

MALLOC_DECLARE(M_FILEMON);
MALLOC_DEFINE(M_FILEMON, "filemon", "File access monitor");

struct filemon_proc {
	TAILQ_ENTRY(filemon_proc) proc; /* List of procs for this filemon. */
	struct proc *p;
};

struct filemon {
	TAILQ_HEAD(, filemon_proc) procs; /* Pointer to list of procs. */
	struct sx	lock;		/* Lock mutex for this filemon. */
	struct file	*fp;		/* Output file pointer. */
	char		fname1[MAXPATHLEN]; /* Temporary filename buffer. */
	char		fname2[MAXPATHLEN]; /* Temporary filename buffer. */
	char		msgbufr[1024];	/* Output message buffer. */
	int		refcnt;		/* In-use refcount. */
};

static struct cdev *filemon_dev;

static void filemon_free(struct filemon *);
static void filemon_destroy(struct filemon *);
static void filemon_track_process(struct filemon *, struct proc *);
static void filemon_untrack_process(struct filemon *, struct proc *);

#include "filemon_wrapper.c"

static void
filemon_comment(struct filemon *filemon)
{
	int len;
	struct timeval now;

	getmicrotime(&now);

	len = snprintf(filemon->msgbufr, sizeof(filemon->msgbufr),
	    "# filemon version %d\n# Target pid %d\n# Start %ju.%06ju\nV %d\n",
	    FILEMON_VERSION, curproc->p_pid, (uintmax_t)now.tv_sec,
	    (uintmax_t)now.tv_usec, FILEMON_VERSION);

	filemon_output(filemon, filemon->msgbufr, len);
}

static void
filemon_free(struct filemon *filemon)
{
	size_t len;
	struct timeval now;

	if (filemon->fp != NULL) {
		getmicrotime(&now);

		len = snprintf(filemon->msgbufr,
		    sizeof(filemon->msgbufr),
		    "# Stop %ju.%06ju\n# Bye bye\n",
		    (uintmax_t)now.tv_sec, (uintmax_t)now.tv_usec);

		filemon_output(filemon, filemon->msgbufr, len);
	} else
		fdrop(filemon->fp, curthread);

	sx_xunlock(&filemon->lock);
	sx_destroy(&filemon->lock);
	free(filemon, M_FILEMON);
}

static void
filemon_destroy(struct filemon *filemon)
{

	sx_assert(&filemon->lock, SA_XLOCKED);

	if (filemon->refcnt == 0)
		filemon_free(filemon);
	else
		sx_xunlock(&filemon->lock);
}

static void
filemon_track_process(struct filemon *filemon, struct proc *p)
{
	struct filemon_proc *filemon_proc;

	sx_assert(&filemon->lock, SA_XLOCKED);
	PROC_LOCK_ASSERT(p, MA_OWNED);

	filemon_proc = malloc(sizeof(struct filemon_proc), M_FILEMON,
	    M_WAITOK | M_ZERO);
	filemon_proc->p = p;
	TAILQ_INSERT_TAIL(&filemon->procs, filemon_proc, proc);

	p->p_filemon = filemon;
}

static void
filemon_untrack_process(struct filemon *filemon, struct proc *p)
{
	struct filemon_proc *filemon_proc;

	sx_assert(&filemon->lock, SA_XLOCKED);
	PROC_LOCK_ASSERT(p, MA_OWNED);

	TAILQ_FOREACH(filemon_proc, &filemon->procs, proc) {
		if (filemon_proc->p == p) {
			TAILQ_REMOVE(&filemon->procs, filemon_proc, proc);
			free(filemon_proc, M_FILEMON);
			break;
		}
	}

	p->p_filemon = NULL;
}

/*
 * Invalidate the passed filemon in all processes.
 * A NULL filemon will invalidate all filemons for all processes.
 */
static int
filemon_invalidate_procs(struct filemon *filemon)
{
	int error;
	struct filemon *p_filemon;
	struct proc *p;

	error = 0;
	if (filemon != NULL)
		sx_assert(&filemon->lock, SA_XLOCKED);
	sx_slock(&allproc_lock);
	FOREACH_PROC_IN_SYSTEM(p) {
		PROC_LOCK(p);
		p_filemon = NULL;
		if (filemon == NULL && p->p_filemon != NULL) {
			/*
			 * When invalidating all, consider immediate lock
			 * failures to be EBUSY.
			 */
			if (sx_try_xlock(&p->p_filemon->lock))
				p_filemon = p->p_filemon;
			else
				error = EBUSY;
		} else if (filemon == p->p_filemon) {
			/* filemon already locked. */
			p_filemon = p->p_filemon;
		}
		if (p_filemon != NULL) {
			--p_filemon->refcnt;
			filemon_untrack_process(p_filemon, p);
			if (filemon == NULL)
				filemon_destroy(p_filemon);
		}
		PROC_UNLOCK(p);
	}
	sx_sunlock(&allproc_lock);

	return (error);
}


static void
filemon_dtr(void *data)
{
	struct filemon *filemon = data;

	if (filemon == NULL)
		return;

	sx_xlock(&filemon->lock);
	filemon_invalidate_procs(filemon);
	/* filemon_invalidate_procs decrements refcnt. */
	filemon_destroy(filemon);
}

static int
filemon_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag __unused,
    struct thread *td)
{
	int error = 0;
	struct filemon *filemon;
	struct proc *p;
	cap_rights_t rights;

	if ((error = devfs_get_cdevpriv((void **) &filemon)) != 0)
		return (error);

	sx_xlock(&filemon->lock);

	switch (cmd) {
	/* Set the output file descriptor. */
	case FILEMON_SET_FD:
		if (filemon->fp != NULL)
			fdrop(filemon->fp, td);

		error = fget_write(td, *(int *)data,
		    cap_rights_init(&rights, CAP_PWRITE),
		    &filemon->fp);
		if (error == 0)
			/* Write the file header. */
			filemon_comment(filemon);
		break;

	/* Set the monitored process ID. */
	case FILEMON_SET_PID:
		/* Invalidate any existing processes already set. */
		filemon_invalidate_procs(filemon);

		error = pget(*((pid_t *)data), PGET_CANDEBUG | PGET_NOTWEXIT,
		    &p);
		if (error == 0) {
			filemon_track_process(filemon, p);
			PROC_UNLOCK(p);
		}
		break;

	default:
		error = EINVAL;
		break;
	}

	sx_xunlock(&filemon->lock);
	return (error);
}

static int
filemon_open(struct cdev *dev, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
	int error;
	struct filemon *filemon;

	filemon = malloc(sizeof(struct filemon), M_FILEMON,
	    M_WAITOK | M_ZERO);
	sx_init(&filemon->lock, "filemon");
	filemon->refcnt = 1;
	TAILQ_INIT(&filemon->procs);

	error = devfs_set_cdevpriv(filemon, filemon_dtr);
	if (error != 0)
		filemon_free(filemon);

	return (error);
}

static int
filemon_close(struct cdev *dev __unused, int flag __unused, int fmt __unused,
    struct thread *td __unused)
{

	return (0);
}

static void
filemon_load(void *dummy __unused)
{

	/* Install the syscall wrappers. */
	filemon_wrapper_install();

	filemon_dev = make_dev(&filemon_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666,
	    "filemon");
}

static int
filemon_unload(void)
{
	int error;

	error = filemon_invalidate_procs(NULL);
	if (error != 0)
		return (error);
	destroy_dev(filemon_dev);
	filemon_wrapper_deinstall();

	return (0);
}

static int
filemon_modevent(module_t mod __unused, int type, void *data)
{
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		filemon_load(data);
		break;

	case MOD_UNLOAD:
		error = filemon_unload();
		break;

	case MOD_QUIESCE:
		error = filemon_invalidate_procs(NULL);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		error = EOPNOTSUPP;
		break;

	}

	return (error);
}

DEV_MODULE(filemon, filemon_modevent, NULL);
MODULE_VERSION(filemon, 1);
