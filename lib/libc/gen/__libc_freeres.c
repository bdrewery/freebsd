/*-
 * Copyright (c) 2015 EMC / Isilon Storage Division
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

#include <sys/param.h>
#include <sys/linker_set.h>
#include <machine/atomic.h>
#include <assert.h>
#include <stdlib.h>
#include "libc_private.h"

/* Callbacks registered in linker set via _LIBC_FREERES_REGISTER(). */
SET_DECLARE(freeres_set, freeres_cb_t);
/*
 * Externally loaded libraries can call _libc_freeres_register().  This list
 * is fixed-size as malloc(3) usage would cause __libc_freeres() to register
 * as a leak, which defeats the purpose.
 */
#define FREERES_REGISTER_FIXED_SIZE	10
static freeres_cb_t *freeres_list[FREERES_REGISTER_FIXED_SIZE];
static int in_freeres;

void
__libc_freeres(void)
{
	freeres_cb_t * const *cb;
	int i;

	/*
	 * Some of the cleanup functions destroy mutexes.  Do not allow
	 * multiple threads in here to help avoid locking issues down the line.
	 */
	if (atomic_cmpset_acq_int(&in_freeres, 0, 1) == 0)
		return;
	/* Call local libc set-loaded callbacks. */
	SET_FOREACH(cb, freeres_set)
		(*cb)();
	/* Call dynamically registered callbacks. */
	for (i = 0; i < FREERES_REGISTER_FIXED_SIZE; i++) {
		if (freeres_list[i] != NULL)
			freeres_list[i]();
	}
	atomic_store_rel_int(&in_freeres, 0);
}

void
_libc_freeres_register(freeres_cb_t *cb)
{
	int i;

	for (i = 0; i <= FREERES_REGISTER_FIXED_SIZE &&
	    freeres_list[i] != NULL; i++)
		;
	assert(i != FREERES_REGISTER_FIXED_SIZE);
	freeres_list[i] = cb;
}

void
_libc_freeres_unregister(freeres_cb_t *cb)
{
	int i;

	for (i = 0; i < FREERES_REGISTER_FIXED_SIZE; i++) {
		if (freeres_list[i] != NULL &&
		    freeres_list[i] == cb) {
			freeres_list[i] = NULL;
			break;
		}
	}
}
