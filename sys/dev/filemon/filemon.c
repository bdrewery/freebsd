/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2011, David E. O'Brien.
 * Copyright (c) 2009-2011, Juniper Networks, Inc.
 * Copyright (c) 2015-2016, EMC Corp.
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

#include <sys/param.h>
#include <sys/file.h>
#include <sys/systm.h>
#include <sys/capsicum.h>
#include <sys/conf.h>
#define _FILEMON_PRIVATE
#include <sys/filemon.h>
#include <sys/fcntl.h>
#include <sys/ioccom.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/sx.h>

#include "filemon.h"

static d_close_t	filemon_close;
static d_ioctl_t	filemon_ioctl;
static d_open_t		filemon_open;

static struct cdevsw filemon_cdevsw = {
	.d_version	= D_VERSION,
	.d_close	= filemon_close,
	.d_ioctl	= filemon_ioctl,
	.d_open		= filemon_open,
	.d_name		= "filemon",
};

static struct cdev *filemon_dev;

/*
 * The devfs file is being closed.  Untrace all processes.  It is possible
 * filemon_close/close(2) was not called.
 */
static void
filemon_dtr(void *data)
{
	struct filemon *filemon = data;

	if (filemon == NULL)
		return;

	sx_xlock(&filemon->lock);
	/*
	 * Detach the filemon.  It cannot be inherited after this.
	 */
	filemon_untrack_processes(filemon);
	filemon_close_log(filemon);
	filemon_drop(filemon);
}

static int
filemon_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int flag __unused,
    struct thread *td)
{
	int error = 0;
	struct filemon *filemon;
	struct proc *p;

	if ((error = devfs_get_cdevpriv((void **) &filemon)) != 0)
		return (error);

	sx_xlock(&filemon->lock);

	switch (cmd) {
	/* Set the output file descriptor. */
	case FILEMON_SET_FD:
		if (filemon->fp != NULL) {
			error = EEXIST;
			break;
		}

		error = fget_write(td, *(int *)data,
		    &cap_pwrite_rights,
		    &filemon->fp);
		if (error == 0)
			/* Write the file header. */
			filemon_write_header(filemon);
		break;

	/* Set the monitored process ID. */
	case FILEMON_SET_PID:
		/* Invalidate any existing processes already set. */
		filemon_untrack_processes(filemon);

		error = pget(*((pid_t *)data),
		    PGET_CANDEBUG | PGET_NOTWEXIT | PGET_NOTINEXEC, &p);
		if (error == 0) {
			KASSERT(p->p_filemon != filemon,
			    ("%s: proc %p didn't untrack filemon %p",
			    __func__, p, filemon));
			error = filemon_attach_proc(filemon, p);
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
    struct thread *td)
{
	int error;
	struct filemon *filemon;

	filemon = malloc(sizeof(*filemon), M_FILEMON,
	    M_WAITOK | M_ZERO);
	sx_init(&filemon->lock, "filemon");
	refcount_init(&filemon->refcnt, 1);
	filemon->cred = crhold(td->td_ucred);

	error = devfs_set_cdevpriv(filemon, filemon_dtr);
	if (error != 0)
		filemon_release(filemon);

	return (error);
}

/* Called on close of last devfs file handle, before filemon_dtr(). */
static int
filemon_close(struct cdev *dev __unused, int flag __unused, int fmt __unused,
    struct thread *td __unused)
{
	struct filemon *filemon;
	int error;

	if ((error = devfs_get_cdevpriv((void **) &filemon)) != 0)
		return (error);

	sx_xlock(&filemon->lock);
	filemon_close_log(filemon);
	error = filemon->error;
	sx_xunlock(&filemon->lock);
	/*
	 * Processes are still being traced but won't log anything
	 * now.  After this call returns filemon_dtr() is called which
	 * will detach processes.
	 */

	return (error);
}

static void
filemon_load(void *dummy __unused)
{

	filemon_dev = make_dev(&filemon_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666,
	    "filemon");
}

static int
filemon_unload(void)
{

	destroy_dev(filemon_dev);

	return (0);
}

static int
filemon_modevent(module_t mod __unused, int type, void *data)
{
	int error = 0;

	switch (type) {
#ifdef FILEMON_HOOKS
	case MOD_LOAD:
		filemon_load(data);
		break;

	case MOD_UNLOAD:
		error = filemon_unload();
		break;

	case MOD_QUIESCE:
		/* XXX: This can work now after detaching all */
		error = EBUSY;
		break;

	case MOD_SHUTDOWN:
		break;
#endif
	default:
#ifndef FILEMON_HOOKS
		printf("filemon(4) requires kernel built with FILEMON_HOOKS\n");
#endif
		error = EOPNOTSUPP;
		break;

	}

	return (error);
}

DEV_MODULE(filemon, filemon_modevent, NULL);
MODULE_VERSION(filemon, 1);
