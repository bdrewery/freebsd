/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2018 Bryan Drewery <bdrewery@FreeBSD.org>
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/queue.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <spawn.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"
#include "libc_private.h"

int
freebsd11_posix_spawn(pid_t *pid, const char *path,
    const _freebsd11_posix_spawn_file_actions_t *fa,
    const _freebsd11_posix_spawnattr_t *sa,
    char * const argv[], char * const envp[])
{

	return (posix_spawn(pid, path, *fa, *sa, argv, envp));
}

int
freebsd11_posix_spawnp(pid_t *pid, const char *path,
    const _freebsd11_posix_spawn_file_actions_t *fa,
    const _freebsd11_posix_spawnattr_t *sa,
    char * const argv[], char * const envp[])
{

	return (posix_spawnp(pid, path, *fa, *sa, argv, envp));
}

/*
 * File descriptor actions
 */

int
freebsd11_posix_spawn_file_actions_init(_freebsd11_posix_spawn_file_actions_t *ret)
{
	_freebsd11_posix_spawn_file_actions_t fa;

	fa = malloc(sizeof(struct posix_spawn_file_actions_t));
	if (fa == NULL)
		return (-1);

	*ret = fa;
	return (posix_spawn_file_actions_init(*ret));
}

int
freebsd11_posix_spawn_file_actions_destroy(_freebsd11_posix_spawn_file_actions_t *fa)
{
	int ret;

	ret = posix_spawn_file_actions_destroy(*fa);
	free(*fa);
	return (ret);
}

int
freebsd11_posix_spawn_file_actions_addopen(_freebsd11_posix_spawn_file_actions_t * __restrict fa,
    int fildes, const char * __restrict path, int oflag, mode_t mode)
{

	return (posix_spawn_file_actions_addopen(*fa, fildes,
	    path, oflag, mode));
}

int
freebsd11_posix_spawn_file_actions_adddup2(_freebsd11_posix_spawn_file_actions_t *fa,
    int fildes, int newfildes)
{

	return (posix_spawn_file_actions_adddup2(*fa, fildes, newfildes));
}

int
freebsd11_posix_spawn_file_actions_addclose(_freebsd11_posix_spawn_file_actions_t *fa,
    int fildes)
{

	return (posix_spawn_file_actions_addclose(*fa, fildes));
}

/*
 * Spawn attributes
 */

int
freebsd11_posix_spawnattr_init(_freebsd11_posix_spawnattr_t *ret)
{
	_freebsd11_posix_spawnattr_t sa;

	sa = malloc(sizeof(struct posix_spawnattr_t));
	if (sa == NULL)
		return (errno);

	/* Set defaults as specified by POSIX, cleared above */
	*ret = sa;
	return (posix_spawnattr_init(*ret));
}

int
freebsd11_posix_spawnattr_destroy(_freebsd11_posix_spawnattr_t *sa)
{
	int ret;

	ret = posix_spawnattr_destroy(*sa);
	free(*sa);
	return (ret);
}

int
freebsd11_posix_spawnattr_getflags(const _freebsd11_posix_spawnattr_t * __restrict sa,
    short * __restrict flags)
{

	return (posix_spawnattr_getflags(*sa, flags));
}

int
freebsd11_posix_spawnattr_getpgroup(const _freebsd11_posix_spawnattr_t * __restrict sa,
    pid_t * __restrict pgroup)
{

	return (posix_spawnattr_getpgroup(*sa, pgroup));
}

int
freebsd11_posix_spawnattr_getschedparam(const _freebsd11_posix_spawnattr_t * __restrict sa,
    struct sched_param * __restrict schedparam)
{

	return (posix_spawnattr_getschedparam(*sa, schedparam));
}

int
freebsd11_posix_spawnattr_getschedpolicy(const _freebsd11_posix_spawnattr_t * __restrict sa,
    int * __restrict schedpolicy)
{

	return (posix_spawnattr_getschedpolicy(*sa, schedpolicy));
}

int
freebsd11_posix_spawnattr_getsigdefault(const _freebsd11_posix_spawnattr_t * __restrict sa,
    sigset_t * __restrict sigdefault)
{

	return (posix_spawnattr_getsigdefault(*sa, sigdefault));
}

int
freebsd11_posix_spawnattr_getsigmask(const _freebsd11_posix_spawnattr_t * __restrict sa,
    sigset_t * __restrict sigmask)
{

	return (posix_spawnattr_getsigmask(*sa, sigmask));
}

int
freebsd11_posix_spawnattr_setflags(_freebsd11_posix_spawnattr_t *sa, short flags)
{

	return (posix_spawnattr_setflags(*sa, flags));
}

int
freebsd11_posix_spawnattr_setpgroup(_freebsd11_posix_spawnattr_t *sa, pid_t pgroup)
{

	return (posix_spawnattr_setpgroup(*sa, pgroup));
}

int
freebsd11_posix_spawnattr_setschedparam(_freebsd11_posix_spawnattr_t * __restrict sa,
    const struct sched_param * __restrict schedparam)
{

	return (posix_spawnattr_setschedparam(*sa, schedparam));
}

int
freebsd11_posix_spawnattr_setschedpolicy(_freebsd11_posix_spawnattr_t *sa, int schedpolicy)
{

	return (posix_spawnattr_setschedpolicy(*sa, schedpolicy));
}

int
freebsd11_posix_spawnattr_setsigdefault(_freebsd11_posix_spawnattr_t * __restrict sa,
    const sigset_t * __restrict sigdefault)
{

	return (posix_spawnattr_setsigdefault(*sa, sigdefault));
}

int
freebsd11_posix_spawnattr_setsigmask(_freebsd11_posix_spawnattr_t * __restrict sa,
    const sigset_t * __restrict sigmask)
{

	return (posix_spawnattr_setsigmask(*sa, sigmask));
}

__sym_compat(posix_spawn, freebsd11_posix_spawn, FBSD_1.1);
__sym_compat(posix_spawn_file_actions_addclose,
    freebsd11_posix_spawn_file_actions_addclose, FBSD_1.1);
__sym_compat(posix_spawn_file_actions_adddup2,
    freebsd11_posix_spawn_file_actions_adddup2, FBSD_1.1);
__sym_compat(posix_spawn_file_actions_addopen,
    freebsd11_posix_spawn_file_actions_addopen, FBSD_1.1);
__sym_compat(posix_spawn_file_actions_destroy,
    freebsd11_posix_spawn_file_actions_destroy, FBSD_1.1);
__sym_compat(posix_spawn_file_actions_init,
    freebsd11_posix_spawn_file_actions_init, FBSD_1.1);
__sym_compat(posix_spawnattr_destroy,
    freebsd11_posix_spawnattr_destroy, FBSD_1.1);
__sym_compat(posix_spawnattr_getflags,
    freebsd11_posix_spawnattr_getflags, FBSD_1.1);
__sym_compat(posix_spawnattr_getpgroup,
    freebsd11_posix_spawnattr_getpgroup, FBSD_1.1);
__sym_compat(posix_spawnattr_getschedparam,
    freebsd11_posix_spawnattr_getschedparam, FBSD_1.1);
__sym_compat(posix_spawnattr_getschedpolicy,
    freebsd11_posix_spawnattr_getschedpolicy, FBSD_1.1);
__sym_compat(posix_spawnattr_getsigdefault,
    freebsd11_posix_spawnattr_getsigdefault, FBSD_1.1);
__sym_compat(posix_spawnattr_getsigmask,
    freebsd11_posix_spawnattr_getsigmask, FBSD_1.1);
__sym_compat(posix_spawnattr_init,
    freebsd11_posix_spawnattr_init, FBSD_1.1);
__sym_compat(posix_spawnattr_setflags,
    freebsd11_posix_spawnattr_setflags, FBSD_1.1);
__sym_compat(posix_spawnattr_setpgroup,
    freebsd11_posix_spawnattr_setpgroup, FBSD_1.1);
__sym_compat(posix_spawnattr_setschedparam,
    freebsd11_posix_spawnattr_setschedparam, FBSD_1.1);
__sym_compat(posix_spawnattr_setschedpolicy,
    freebsd11_posix_spawnattr_setschedpolicy, FBSD_1.1);
__sym_compat(posix_spawnattr_setsigdefault,
    freebsd11_posix_spawnattr_setsigdefault, FBSD_1.1);
__sym_compat(posix_spawnattr_setsigmask,
    freebsd11_posix_spawnattr_setsigmask, FBSD_1.1);
__sym_compat(posix_spawnp, freebsd11_posix_spawnp, FBSD_1.1);
