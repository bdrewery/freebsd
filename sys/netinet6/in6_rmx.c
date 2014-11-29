/*-
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$KAME: in6_rmx.c,v 1.11 2001/07/26 06:53:16 jinmei Exp $
 */

/*-
 * Copyright 1994, 1995 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/rwlock.h>
#include <sys/syslog.h>
#include <sys/callout.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/route.h>
#include <net/route_internal.h>

#include <netinet/in.h>
#include <netinet/ip_var.h>
#include <netinet/in_var.h>

#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>

#include <netinet/icmp6.h>
#include <netinet6/nd6.h>

#include <netinet/tcp.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

extern int	in6_inithead(void **head, int off);
#ifdef VIMAGE
extern int	in6_detachhead(void **head, int off);
#endif

/*
 * Do what we need to do when inserting a route.
 */
static struct radix_node *
in6_addroute(void *v_arg, void *n_arg, struct radix_head *head,
    struct radix_node *treenodes)
{
	unsigned int mtu, rt_flags;
	struct rtentry *rt;
	const struct sockaddr_in6 *sin6;
	struct ifnet *ifp;
	struct ifaddr *ifa;

	rt = (struct rtentry *)treenodes;
	sin6 = (const struct sockaddr_in6 *)rte_get_dst(rt);
	rt_flags = rte_get_flags(rt);
	ifp = rte_get_lifp(rt);
	ifa = rte_get_ifa(rt);
	
	struct radix_node *ret;

	if (IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr))
		rt_flags |= RTF_MULTICAST;

	/*
	 * A little bit of help for both IPv6 output and input:
	 *   For local addresses, we make sure that RTF_LOCAL is set,
	 *   with the thought that this might one day be used to speed up
	 *   ip_input().
	 *
	 * We also mark routes to multicast addresses as such, because
	 * it's easy to do and might be useful (but this is much more
	 * dubious since it's so easy to inspect the address).  (This
	 * is done above.)
	 *
	 * XXX
	 * should elaborate the code.
	 */
	if (rt_flags & RTF_HOST) {
		if (IN6_ARE_ADDR_EQUAL(&satosin6(ifa->ifa_addr)->sin6_addr,
		    &sin6->sin6_addr)) {
			rt_flags |= RTF_LOCAL;
		}
	}

	rte_set_flags(rt, rt_flags);

	if (ifp != NULL) {

		/*
		 * Check route MTU:
		 * inherit interface MTU if not set or
		 * check if MTU is too large.
		 */
		mtu = rte_get_mtu(rt);
		if (mtu == 0) {
			rte_set_mtu(rt, IN6_LINKMTU(rt->rt_ifp));
		} else if (mtu > IN6_LINKMTU(rt->rt_ifp))
			rte_set_mtu(rt, IN6_LINKMTU(rt->rt_ifp));
	}

	ret = rn_addroute(v_arg, n_arg, head, treenodes);
	if (ret == NULL) {
		struct rtentry *rt2;
		struct sockaddr *gw;
		/*
		 * We are trying to add a net route, but can't.
		 * The following case should be allowed, so we'll make a
		 * special check for this:
		 *	Two IPv6 addresses with the same prefix is assigned
		 *	to a single interrface.
		 *	# ifconfig if0 inet6 3ffe:0501::1 prefix 64 alias (*1)
		 *	# ifconfig if0 inet6 3ffe:0501::2 prefix 64 alias (*2)
		 *	In this case, (*1) and (*2) want to add the same
		 *	net route entry, 3ffe:0501:: -> if0.
		 *	This case should not raise an error.
		 */
		rt2 = in6_rtalloc1((struct sockaddr *)sin6, 0, RTF_RNH_LOCKED,
		    rt->rt_fibnum);
		if (rt2) {
			rt_flags = rte_get_flags(rt2);
			ifp = rte_get_lifp(rt2);
			gw = rte_get_gw(rt2);
			if (((rt_flags & (RTF_HOST|RTF_GATEWAY)) == 0)
			 && gw != NULL 
			 && gw->sa_family == AF_LINK
			 && ifp == rte_get_lifp(rt)) {
				ret = rt2->rt_nodes;
			}
			RTFREE_LOCKED(rt2);
		}
	}
	return (ret);
}

/*
 * Initialize our routing tree.
 */
static VNET_DEFINE(int, _in6_rt_was_here);
#define	V__in6_rt_was_here	VNET(_in6_rt_was_here)

int
in6_inithead(void **head, int off)
{
	struct rib_head *rh;

	rh = rt_table_init(offsetof(struct sockaddr_in6, sin6_addr) << 3);
	if (rh == NULL)
		return (0);

	rh->rnh_addaddr = in6_addroute;
	*head = (void *)rh;

	if (V__in6_rt_was_here == 0)
		V__in6_rt_was_here = 1;

	return (1);
}

#ifdef VIMAGE
int
in6_detachhead(void **head, int off)
{

	callout_drain(&V_rtq_mtutimer);
	return (1);
}
#endif

/*
 * Extended API for IPv6 FIB support.
 */
void
in6_rtredirect(struct sockaddr *dst, struct sockaddr *gw, struct sockaddr *nm,
    int flags, struct sockaddr *src, u_int fibnum)
{

	rtredirect_fib(dst, gw, nm, flags, src, fibnum);
}

int
in6_rtrequest(int req, struct sockaddr *dst, struct sockaddr *gw,
    struct sockaddr *mask, int flags, struct rtentry **ret_nrt, u_int fibnum)
{

	return (rtrequest_fib(req, dst, gw, mask, flags, ret_nrt, fibnum));
}

struct rtentry *
in6_rtalloc1(struct sockaddr *dst, int report, u_long ignflags, u_int fibnum)
{

	return (rtalloc1_fib(dst, report, ignflags, fibnum));
}
