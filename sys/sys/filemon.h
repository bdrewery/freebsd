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

#ifndef _SYS_FILEMON_H
#define _SYS_FILEMON_H
#ifdef _KERNEL

#include <sys/param.h>
#include <sys/uio.h>

#define FILEMON_VERSION		5	/* output format
					   (bump when adding record types) */

struct image_params;
struct proc;
#ifdef _FILEMON_PRIVATE
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/refcount.h>
#include <sys/sx.h>

MALLOC_DECLARE(M_FILEMON);

/*
 * The filemon->lock protects several things currently:
 * - fname1/fname2/msgbufr are pre-allocated and used per syscall
 *   for logging and copyins rather than stack variables.
 * - Serializing the filemon's log output.
 * - Preventing inheritance or removal of the filemon into proc.p_filemon.
 */
struct sx;
struct file;
struct ucred;
struct filemon {
	struct sx	lock;		/* Lock for this filemon. */
	struct file	*fp;		/* Output file pointer. */
	struct ucred	*cred;		/* Credential of tracer. */
	char		fname1[MAXPATHLEN]; /* Temporary filename buffer. */
	char		fname2[MAXPATHLEN]; /* Temporary filename buffer. */
	char		msgbufr[2*MAXPATHLEN + 100];	/* Output message buffer. */
	int		error;		/* Log write error, returned on close(2). */
	u_int		refcnt;		/* Pointer reference count. */
	u_int		proccnt;	/* Process count. */
};

int		filemon_attach_proc(struct filemon *filemon, struct proc *p);
void		filemon_untrack_processes(struct filemon *filemon);
void		filemon_output(struct filemon *filemon, char *msg, size_t len);
void		filemon_release(struct filemon *filemon);
void		filemon_write_header(struct filemon *);
void		filemon_close_log(struct filemon *filemon);

static __inline struct filemon *
filemon_acquire(struct filemon *filemon)
{

	if (filemon != NULL)
		refcount_acquire(&filemon->refcnt);
	return (filemon);
}

/* Unlock and release the filemon. */
static __inline void
filemon_drop(struct filemon *filemon)
{

	sx_xunlock(&filemon->lock);
	filemon_release(filemon);
}
#endif

#ifdef FILEMON_HOOKS
#define FILEMON_HAVE_HOOKS()	1
#define	FILEMON_ENABLED(p)	__predict_false(p->p_filemon != NULL)

void	filemon_hook_chdir(struct proc *p, const char *path,
    enum uio_seg pathseg);
void	filemon_hook_renameat(struct proc *p, int oldfd, const char *old,
    int newfd, const char *new, enum uio_seg pathseg);
void	filemon_hook_linkat(struct proc *p, int fd1, const char *path1,
    int fd2, const char *path2, enum uio_seg pathseg, int follow);
void	filemon_hook_symlinkat(struct proc *p, const char *path1, int fd,
    const char *path2, enum uio_seg pathseg);
void	filemon_hook_unlinkat(struct proc *p, int dfd, const char *path,
    int fd, int flag, enum uio_seg pathseg);
void	filemon_hook_openat(struct proc *p, int fd, const char *path,
    enum uio_seg pathseg, int flags);
void	filemon_hook_exit(struct proc *p);
void	filemon_hook_exec(struct proc *p, struct image_params *imgp);
void	filemon_hook_fork(struct proc *p1, struct proc *p2);
#else
#define FILEMON_HAVE_HOOKS()	0
#endif	/* FILEMON_HOOKS */

#endif	/* _KERNEL */
#endif	/* _SYS_FILEMON_H */
