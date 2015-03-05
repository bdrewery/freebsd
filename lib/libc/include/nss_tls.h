/*-
 * Copyright (c) 2003 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by
 * Jacques A. Vidrine, Safeport Network Services, and Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035
 * ("CBOSS"), as part of the DARPA CHATS research program.
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
 *
 * Macros which generate thread local storage handling code in NSS modules.
 */
#ifndef _NSS_TLS_H_
#define _NSS_TLS_H_

#define NSS_TLS_HANDLING(name)					\
static pthread_key_t name##_state_key;				\
static	void	 name##_keyinit(void);				\
static	int	 name##_getstate(struct name##_state **);	\
\
static void								\
name##_keyinit(void)							\
{									\
	(void)_pthread_key_create(&name##_state_key, name##_endstate);	\
}									\
\
static pthread_once_t name##_once = PTHREAD_ONCE_INIT;		\
static struct name##_state name##_gstate;			\
static int							\
name##_getstate(struct name##_state **p)			\
{								\
	int			rv;				\
								\
	if (!__isthreaded || _pthread_main_np() != 0) {		\
		*p = &name##_gstate;				\
		return (0);					\
	}							\
	rv = _pthread_once(&name##_once, name##_keyinit);	\
	if (rv != 0)						\
		return (rv);					\
	*p = _pthread_getspecific(name##_state_key);		\
	if (*p != NULL)						\
		return (0);					\
	*p = calloc(1, sizeof(**p));				\
	if (*p == NULL)						\
		return (ENOMEM);				\
	rv = _pthread_setspecific(name##_state_key, *p);	\
	if (rv != 0) {						\
		free(*p);					\
		*p = NULL;					\
	}							\
	return (rv);						\
}								\
/* allow the macro invocation to end with a semicolon */	\
struct _clashproof_bmVjdGFy

#endif /* _NSS_TLS_H_ */
