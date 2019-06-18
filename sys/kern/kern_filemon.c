/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2011, David E. O'Brien.
 * Copyright (c) 2009-2011, Juniper Networks, Inc.
 * Copyright (c) 2015-2019, Dell EMC
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
#include <sys/buf.h>
#include <sys/capsicum.h>
#include <sys/filedesc.h>
#define _FILEMON_PRIVATE
#include <sys/filemon.h>
#include <sys/imgact.h>
#include <sys/malloc.h>
#include <sys/priv.h>
#include <sys/sx.h>
#include <sys/sysent.h>
#include <sys/vnode.h>

#include <machine/stdarg.h>

/* XXX: convert to sbuf */

MALLOC_DEFINE(M_FILEMON, "filemon", "File access monitor");
static void		filemon_output_event(struct filemon *filemon,
    const char *fmt, ...) __printflike(2, 3);
static struct filemon	*filemon_proc_get(struct proc *p);
static void		filemon_proc_drop(struct proc *p);

/* Attach the filemon to the process. */
int
filemon_attach_proc(struct filemon *filemon, struct proc *p)
{
	struct filemon *filemon2;

	sx_assert(&filemon->lock, SA_XLOCKED);
	PROC_LOCK_ASSERT(p, MA_OWNED);
	KASSERT((p->p_flag & P_WEXIT) == 0,
	    ("%s: filemon %p attaching to exiting process %p",
	    __func__, filemon, p));
	KASSERT((p->p_flag & P_INEXEC) == 0,
	    ("%s: filemon %p attaching to execing process %p",
	    __func__, filemon, p));

	if (p->p_filemon == filemon)
		return (0);
	/*
	 * Don't allow truncating other process traces.  It is
	 * not really intended to trace procs other than curproc
	 * anyhow.
	 */
	if (p->p_filemon != NULL && p != curproc)
		return (EBUSY);
	/*
	 * Historic behavior of filemon has been to let a child initiate
	 * tracing on itself and cease existing tracing.  Bmake
	 * .META + .MAKE relies on this.  It is only relevant for attaching to
	 * curproc.
	 */
	while (p->p_filemon != NULL) {
		PROC_UNLOCK(p);
		sx_xunlock(&filemon->lock);
		while ((filemon2 = filemon_proc_get(p)) != NULL) {
			/* It may have changed. */
			if (p->p_filemon == filemon2)
				filemon_proc_drop(p);
			filemon_drop(filemon2);
		}
		sx_xlock(&filemon->lock);
		PROC_LOCK(p);
		/*
		 * It may have been attached to, though unlikely.
		 * Try again if needed.
		 */
	}

	KASSERT(p->p_filemon == NULL,
	    ("%s: proc %p didn't detach filemon %p", __func__, p,
	    p->p_filemon));
	p->p_filemon = filemon_acquire(filemon);
	++filemon->proccnt;

	return (0);
}

/*
 * Invalidate the passed filemon in all processes.
 */
void
filemon_untrack_processes(struct filemon *filemon)
{
	struct proc *p;

	sx_assert(&filemon->lock, SA_XLOCKED);

	/* Avoid allproc loop if there is no need. */
	if (filemon->proccnt == 0)
		return;

	/*
	 * Processes in this list won't go away while here since
	 * filemon_event_process_exit() will lock on filemon->lock
	 * which we hold.
	 */
	sx_slock(&allproc_lock);
	FOREACH_PROC_IN_SYSTEM(p) {
		/*
		 * No PROC_LOCK is needed to compare here since it is
		 * guaranteed to not change since we have its filemon
		 * locked.  Everything that changes this p_filemon will
		 * be locked on it.
		 */
		if (p->p_filemon == filemon)
			filemon_proc_drop(p);
	}
	sx_sunlock(&allproc_lock);

	/*
	 * It's possible some references were acquired but will be
	 * dropped shortly as they are restricted from being
	 * inherited.  There is at least the reference in cdevpriv remaining.
	 */
	KASSERT(filemon->refcnt > 0, ("%s: filemon %p should have "
	    "references still.", __func__, filemon));
	KASSERT(filemon->proccnt == 0, ("%s: filemon %p should not have "
	    "attached procs still.", __func__, filemon));
}

/*
 * Close out the log.
 */
void
filemon_close_log(struct filemon *filemon)
{
	struct file *fp;
	struct timeval now;
	size_t len;

	sx_assert(&filemon->lock, SA_XLOCKED);
	if (filemon->fp == NULL)
		return;

	getmicrotime(&now);

	len = snprintf(filemon->msgbufr,
	    sizeof(filemon->msgbufr),
	    "# Stop %ju.%06ju\n# Bye bye\n",
	    (uintmax_t)now.tv_sec, (uintmax_t)now.tv_usec);

	if (len < sizeof(filemon->msgbufr))
		filemon_output(filemon, filemon->msgbufr, len);
	fp = filemon->fp;
	filemon->fp = NULL;

	sx_xunlock(&filemon->lock);
	fdrop(fp, curthread);
	sx_xlock(&filemon->lock);
}

/*
 * Release a reference and free on the last one.
 */
void
filemon_release(struct filemon *filemon)
{

	if (refcount_release(&filemon->refcnt) == 0)
		return;
	/*
	 * There are valid cases of releasing while locked, such as in
	 * filemon_untrack_processes, but none which are done where there
	 * is not at least 1 reference remaining.
	 */
	sx_assert(&filemon->lock, SA_UNLOCKED);

	if (filemon->cred != NULL)
		crfree(filemon->cred);
	sx_destroy(&filemon->lock);
	free(filemon, M_FILEMON);
}

/*
 * Acquire the proc's p_filemon reference and lock the filemon.
 * The proc's p_filemon may not match this filemon on return.
 */
static struct filemon *
filemon_proc_get(struct proc *p)
{
	struct filemon *filemon;

	if (p->p_filemon == NULL)
		return (NULL);
	PROC_LOCK(p);
	filemon = filemon_acquire(p->p_filemon);
	PROC_UNLOCK(p);

	if (filemon == NULL)
		return (NULL);
	/*
	 * The p->p_filemon may have changed by now.  That case is handled
	 * by the exit and fork hooks and filemon_attach_proc specially.
	 */
	sx_xlock(&filemon->lock);
	return (filemon);
}

/* Remove and release the filemon on the given process. */
static void
filemon_proc_drop(struct proc *p)
{
	struct filemon *filemon;

	KASSERT(p->p_filemon != NULL, ("%s: proc %p NULL p_filemon",
	    __func__, p));
	sx_assert(&p->p_filemon->lock, SA_XLOCKED);
	PROC_LOCK(p);
	filemon = p->p_filemon;
	p->p_filemon = NULL;
	--filemon->proccnt;
	PROC_UNLOCK(p);
	/*
	 * This should not be the last reference yet.  filemon_release()
	 * cannot be called with filemon locked, which the caller expects
	 * will stay locked.
	 */
	KASSERT(filemon->refcnt > 1, ("%s: proc %p dropping filemon %p "
	    "with last reference", __func__, p, filemon));
	filemon_release(filemon);
}

void
filemon_write_header(struct filemon *filemon)
{
	int len;
	struct timeval now;

	getmicrotime(&now);

	len = snprintf(filemon->msgbufr, sizeof(filemon->msgbufr),
	    "# filemon version %d\n# Target pid %d\n# Start %ju.%06ju\nV %d\n",
	    FILEMON_VERSION, curproc->p_pid, (uintmax_t)now.tv_sec,
	    (uintmax_t)now.tv_usec, FILEMON_VERSION);
	if (len < sizeof(filemon->msgbufr))
		filemon_output(filemon, filemon->msgbufr, len);
}

void
filemon_output(struct filemon *filemon, char *msg, size_t len)
{
	struct uio auio;
	struct iovec aiov;
	int error;

	if (filemon->fp == NULL)
		return;

	aiov.iov_base = msg;
	aiov.iov_len = len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = len;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_td = curthread;
	auio.uio_offset = (off_t) -1;

	if (filemon->fp->f_type == DTYPE_VNODE)
		bwillwrite();

	error = fo_write(filemon->fp, &auio, filemon->cred, 0, curthread);
	if (error != 0 && filemon->error == 0)
		filemon->error = error;
}

static void
filemon_output_event(struct filemon *filemon, const char *fmt, ...)
{
	va_list ap;
	size_t len;

	va_start(ap, fmt);
	len = vsnprintf(filemon->msgbufr, sizeof(filemon->msgbufr), fmt, ap);
	va_end(ap);
	/* The event is truncated but still worth logging. */
	if (len >= sizeof(filemon->msgbufr))
		len = sizeof(filemon->msgbufr) - 1;
	filemon_output(filemon, filemon->msgbufr, len);
}

void
filemon_hook_chdir(struct proc *p, const char *path, enum uio_seg pathseg)
{
	int error;
	struct filemon *filemon;

	if ((filemon = filemon_proc_get(p)) == NULL)
		return;
	/* XXX: Duplicate pattern is annoying */
	if (pathseg == UIO_SYSSPACE)
		error = copystr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	else
		error = copyinstr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}
	filemon_output_event(filemon, "C %d %s\n",
	    p->p_pid, filemon->fname1);
copyfail:
	filemon_drop(filemon);
}

void
filemon_hook_renameat(struct proc *p, int oldfd, const char *old, int newfd,
    const char *new, enum uio_seg pathseg)
{
	int error;
	struct filemon *filemon;

	/* XXX: No support for *at() yet. */
	if (oldfd != AT_FDCWD || newfd != AT_FDCWD)
		return;
	if ((filemon = filemon_proc_get(p)) == NULL)
		return;
	/* XXX: Duplicate pattern is annoying */
	if (pathseg == UIO_SYSSPACE)
		error = copystr(old, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	else
		error = copyinstr(old, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}
	if (pathseg == UIO_SYSSPACE)
		error = copystr(new, filemon->fname2,
		    sizeof(filemon->fname2), NULL);
	else
		error = copyinstr(new, filemon->fname2,
		    sizeof(filemon->fname2), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}

	filemon_output_event(filemon, "M %d '%s' '%s'\n",
	    p->p_pid, filemon->fname1, filemon->fname2);
copyfail:
	filemon_drop(filemon);
}

void
filemon_hook_linkat(struct proc *p, int fd1, const char *path1, int fd2,
    const char *path2, enum uio_seg pathseg, int follow)
{
	int error;
	struct filemon *filemon;

	/* XXX: No support for *at() yet. */
	if (fd1 != AT_FDCWD || fd2 != AT_FDCWD)
		return;
	if ((filemon = filemon_proc_get(p)) == NULL)
		return;
	/* XXX: Duplicate pattern is annoying */
	if (pathseg == UIO_SYSSPACE)
		error = copystr(path1, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	else
		error = copyinstr(path1, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}
	if (pathseg == UIO_SYSSPACE)
		error = copystr(path2, filemon->fname2,
		    sizeof(filemon->fname2), NULL);
	else
		error = copyinstr(path2, filemon->fname2,
		    sizeof(filemon->fname2), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}

	filemon_output_event(filemon, "L %d '%s' '%s'\n",
	    p->p_pid, filemon->fname1, filemon->fname2);
copyfail:
	filemon_drop(filemon);
}

void
filemon_hook_symlinkat(struct proc *p, const char *path1, int fd,
    const char *path2, enum uio_seg pathseg)
{

	filemon_hook_linkat(p, AT_FDCWD, path1, fd, path2, pathseg, 0);
}

/* XXX: Missing rmdir(2) and unlinkat(AT_REMOVEDIR) */
void
filemon_hook_unlinkat(struct proc *p, int dfd, const char *path, int fd,
    __unused int flag, enum uio_seg pathseg)
{
	int error;
	struct filemon *filemon;

	/* XXX: No support for *at() yet. */
	if (dfd != AT_FDCWD || fd != FD_NONE)
		return;
	if ((filemon = filemon_proc_get(p)) == NULL)
		return;
	/* XXX: Duplicate pattern is annoying */
	if (pathseg == UIO_SYSSPACE)
		error = copystr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	else
		error = copyinstr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}

	filemon_output_event(filemon, "D %d %s\n",
	    p->p_pid, filemon->fname1);
copyfail:
	filemon_drop(filemon);
}

void
filemon_hook_openat(struct proc *p, int fd, const char *path,
    enum uio_seg pathseg, int flags)
{
	int error;
	struct file *fp;
	struct filemon *filemon;
	char *atpath, *freepath;
	cap_rights_t rights;

	if ((filemon = filemon_proc_get(p)) == NULL)
		return;

	atpath = "";
	freepath = NULL;
	fp = NULL;

	/* XXX: Duplicate pattern is annoying */
	if (pathseg == UIO_SYSSPACE)
		error = copystr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	else
		error = copyinstr(path, filemon->fname1,
		    sizeof(filemon->fname1), NULL);
	if (error != 0) {
		filemon->error = error;
		goto copyfail;
	}

	if (filemon->fname1[0] != '/' && fd != AT_FDCWD) {
		/*
		 * rats - we cannot do too much about this.
		 * the trace should show a dir we read
		 * recently.. output an A record as a clue
		 * until we can do better.
		 * XXX: This may be able to come out with
		 * the namecache lookup now.
		 */
		filemon_output_event(filemon, "A %d %s\n",
		    p->p_pid, filemon->fname1);
		/*
		 * Try to resolve the path from the vnode using the
		 * namecache.  It may be inaccurate, but better
		 * than nothing.
		 */
		if (getvnode(curthread, fd,
		    cap_rights_init(&rights, CAP_LOOKUP), &fp) == 0) {
			vn_fullpath(curthread, fp->f_vnode, &atpath,
			    &freepath);
		}
	}
	if (flags & O_RDWR) {
		/*
		 * We'll get the W record below, but need
		 * to also output an R to distinguish from
		 * O_WRONLY.
		 */
		filemon_output_event(filemon, "R %d %s%s%s\n",
		    p->p_pid, atpath,
		    atpath[0] != '\0' ? "/" : "", filemon->fname1);
	}

	filemon_output_event(filemon, "%c %d %s%s%s\n",
	    (flags & O_ACCMODE) ? 'W':'R',
	    p->p_pid, atpath,
	    atpath[0] != '\0' ? "/" : "", filemon->fname1);
copyfail:
	filemon_drop(filemon);
	if (fp != NULL)
		fdrop(fp, curthread);
	free(freepath, M_TEMP);
}

void
filemon_hook_exit(struct proc *p)
{
	struct filemon *filemon;

	if ((filemon = filemon_proc_get(p)) == NULL)
		return;

	filemon_output_event(filemon, "X %d %d %d\n",
	    p->p_pid, p->p_xexit, p->p_xsig);

	/*
	 * filemon_untrack_processes() may have dropped this p_filemon
	 * already while in filemon_proc_get() before acquiring the
	 * filemon lock.
	 */
	KASSERT(p->p_filemon == NULL || p->p_filemon == filemon,
	    ("%s: p %p was attached while exiting, expected "
	    "filemon %p or NULL", __func__, p, filemon));
	if (p->p_filemon == filemon)
		filemon_proc_drop(p);

	filemon_drop(filemon);
}

void
filemon_hook_exec(struct proc *p, struct image_params *imgp)
{
	struct filemon *filemon;

	if ((filemon = filemon_proc_get(p)) == NULL)
		return;

	filemon_output_event(filemon, "E %d %s\n",
	    p->p_pid,
	    imgp->execpath != NULL ? imgp->execpath : "<unknown>");

	/* If the credentials changed then cease tracing. */
	if (imgp->newcred != NULL &&
	    imgp->credential_setid &&
	    priv_check_cred(filemon->cred, PRIV_DEBUG_DIFFCRED) != 0) {
		/*
		 * It may have changed to NULL already, but
		 * will not be re-attached by anything else.
		 */
		if (p->p_filemon != NULL) {
			KASSERT(p->p_filemon == filemon,
			    ("%s: proc %p didn't have expected"
			    " filemon %p", __func__, p, filemon));
			filemon_proc_drop(p);
		}
	}

	filemon_drop(filemon);
}

void
filemon_hook_fork(struct proc *p1, struct proc *p2)
{
	struct filemon *filemon;

	if ((filemon = filemon_proc_get(p1)) == NULL)
		return;

	filemon_output_event(filemon, "F %d %d\n",
	    p1->p_pid, p2->p_pid);

	/*
	 * filemon_untrack_processes() or
	 * filemon_ioctl(FILEMON_SET_PID) may have changed the parent's
	 * p_filemon while in filemon_proc_get() before acquiring the
	 * filemon lock.  Only inherit if the parent is still traced by
	 * this filemon.
	 */
	if (p1->p_filemon == filemon) {
		PROC_LOCK(p2);
		/*
		 * It may have been attached to already by a new
		 * filemon.
		 */
		if (p2->p_filemon == NULL) {
			p2->p_filemon = filemon_acquire(filemon);
			++filemon->proccnt;
		}
		PROC_UNLOCK(p2);
	}

	filemon_drop(filemon);
}
