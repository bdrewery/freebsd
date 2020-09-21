/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2020 Alexander V. Chernikov
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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
#include "opt_inet.h"
#include "opt_inet6.h"
#include "opt_mpath.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/rmlock.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/vnet.h>
#include <net/route.h>
#include <net/route/route_ctl.h>
#include <net/route/route_var.h>
#include <net/route/nhop_utils.h>
#include <net/route/nhop.h>
#include <net/route/nhop_var.h>
#include <netinet/in.h>

#ifdef RADIX_MPATH
#include <net/radix_mpath.h>
#endif

#include <vm/uma.h>

/*
 * This file contains control plane routing tables functions.
 *
 * All functions assumes they are called in net epoch.
 */

struct rib_subscription {
	CK_STAILQ_ENTRY(rib_subscription)	next;
	rib_subscription_cb_t			*func;
	void					*arg;
	enum rib_subscription_type		type;
	struct epoch_context			epoch_ctx;
};

static int add_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rib_cmd_info *rc);
static int add_route_nhop(struct rib_head *rnh, struct rtentry *rt,
    struct rt_addrinfo *info, struct route_nhop_data *rnd,
    struct rib_cmd_info *rc);
static int del_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rib_cmd_info *rc);
static int change_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct route_nhop_data *nhd_orig, struct rib_cmd_info *rc);
static int change_route_nhop(struct rib_head *rnh, struct rtentry *rt,
    struct rt_addrinfo *info, struct route_nhop_data *rnd,
    struct rib_cmd_info *rc);

static int rt_unlinkrte(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rib_cmd_info *rc);

static void rib_notify(struct rib_head *rnh, enum rib_subscription_type type,
    struct rib_cmd_info *rc);

static void destroy_subscription_epoch(epoch_context_t ctx);

/* Routing table UMA zone */
VNET_DEFINE_STATIC(uma_zone_t, rtzone);
#define	V_rtzone	VNET(rtzone)

void
vnet_rtzone_init()
{

	V_rtzone = uma_zcreate("rtentry", sizeof(struct rtentry),
		NULL, NULL, NULL, NULL, UMA_ALIGN_PTR, 0);
}

#ifdef VIMAGE
void
vnet_rtzone_destroy()
{

	uma_zdestroy(V_rtzone);
}
#endif

static void
destroy_rtentry(struct rtentry *rt)
{

	/*
	 * At this moment rnh, nh_control may be already freed.
	 * nhop interface may have been migrated to a different vnet.
	 * Use vnet stored in the nexthop to delete the entry.
	 */
	CURVNET_SET(nhop_get_vnet(rt->rt_nhop));

	/* Unreference nexthop */
	nhop_free(rt->rt_nhop);

	uma_zfree(V_rtzone, rt);

	CURVNET_RESTORE();
}

/*
 * Epoch callback indicating rtentry is safe to destroy
 */
static void
destroy_rtentry_epoch(epoch_context_t ctx)
{
	struct rtentry *rt;

	rt = __containerof(ctx, struct rtentry, rt_epoch_ctx);

	destroy_rtentry(rt);
}

/*
 * Schedule rtentry deletion
 */
static void
rtfree(struct rtentry *rt)
{

	KASSERT(rt != NULL, ("%s: NULL rt", __func__));

	epoch_call(net_epoch_preempt, destroy_rtentry_epoch,
	    &rt->rt_epoch_ctx);
}

static struct rib_head *
get_rnh(uint32_t fibnum, const struct rt_addrinfo *info)
{
	struct rib_head *rnh;
	struct sockaddr *dst;

	KASSERT((fibnum < rt_numfibs), ("rib_add_route: bad fibnum"));

	dst = info->rti_info[RTAX_DST];
	rnh = rt_tables_get_rnh(fibnum, dst->sa_family);

	return (rnh);
}

static int
get_info_weight(const struct rt_addrinfo *info, uint32_t default_weight)
{
	uint32_t weight;

	if (info->rti_mflags & RTV_WEIGHT)
		weight = info->rti_rmx->rmx_weight;
	else
		weight = default_weight;
	/* Keep upper 1 byte for adm distance purposes */
	if (weight > RT_MAX_WEIGHT)
		weight = RT_MAX_WEIGHT;

	return (weight);
}

static void
rt_set_expire_info(struct rtentry *rt, const struct rt_addrinfo *info)
{

	/* Kernel -> userland timebase conversion. */
	if (info->rti_mflags & RTV_EXPIRE)
		rt->rt_expire = info->rti_rmx->rmx_expire ?
		    info->rti_rmx->rmx_expire - time_second + time_uptime : 0;
}

/*
 * Check if specified @gw matches gw data in the nexthop @nh.
 *
 * Returns true if matches, false otherwise.
 */
static bool
match_nhop_gw(const struct nhop_object *nh, const struct sockaddr *gw)
{

	if (nh->gw_sa.sa_family != gw->sa_family)
		return (false);

	switch (gw->sa_family) {
	case AF_INET:
		return (nh->gw4_sa.sin_addr.s_addr ==
		    ((const struct sockaddr_in *)gw)->sin_addr.s_addr);
	case AF_INET6:
		{
			const struct sockaddr_in6 *gw6;
			gw6 = (const struct sockaddr_in6 *)gw;

			/*
			 * Currently (2020-09) IPv6 gws in kernel have their
			 * scope embedded. Once this becomes false, this code
			 * has to be revisited.
			 */
			if (IN6_ARE_ADDR_EQUAL(&nh->gw6_sa.sin6_addr,
			    &gw6->sin6_addr))
				return (true);
			return (false);
		}
	case AF_LINK:
		{
			const struct sockaddr_dl *sdl;
			sdl = (const struct sockaddr_dl *)gw;
			return (nh->gwl_sa.sdl_index == sdl->sdl_index);
		}
	default:
		return (memcmp(&nh->gw_sa, gw, nh->gw_sa.sa_len) == 0);
	}

	/* NOTREACHED */
	return (false);
}

/*
 * Checks if data in @info matches nexhop @nh.
 *
 * Returns 0 on success,
 * ESRCH if not matched,
 * ENOENT if filter function returned false
 */
int
check_info_match_nhop(const struct rt_addrinfo *info, const struct rtentry *rt,
    const struct nhop_object *nh)
{
	const struct sockaddr *gw = info->rti_info[RTAX_GATEWAY];

	if (info->rti_filter != NULL) {
	    if (info->rti_filter(rt, nh, info->rti_filterdata) == 0)
		    return (ENOENT);
	    else
		    return (0);
	}
	if ((gw != NULL) && !match_nhop_gw(nh, gw))
		return (ESRCH);

	return (0);
}

/*
 * Checks if nexhop @nh can be rewritten by data in @info because
 *  of higher "priority". Currently the only case for such scenario
 *  is kernel installing interface routes, marked by RTF_PINNED flag.
 *
 * Returns:
 * 1 if @info data has higher priority
 * 0 if priority is the same
 * -1 if priority is lower
 */
int
can_override_nhop(const struct rt_addrinfo *info, const struct nhop_object *nh)
{

	if (info->rti_flags & RTF_PINNED) {
		return (NH_IS_PINNED(nh)) ? 0 : 1;
	} else {
		return (NH_IS_PINNED(nh)) ? -1 : 0;
	}
}

/*
 * Runs exact prefix match based on @dst and @netmask.
 * Returns matched @rtentry if found or NULL.
 * If rtentry was found, saves nexthop / weight value into @rnd.
 */
static struct rtentry *
lookup_prefix_bysa(struct rib_head *rnh, const struct sockaddr *dst,
    const struct sockaddr *netmask, struct route_nhop_data *rnd)
{
	struct rtentry *rt;

	RIB_LOCK_ASSERT(rnh);

	rt = (struct rtentry *)rnh->rnh_lookup(__DECONST(void *, dst),
	    __DECONST(void *, netmask), &rnh->head);
	if (rt != NULL) {
		rnd->rnd_nhop = rt->rt_nhop;
		rnd->rnd_weight = rt->rt_weight;
	} else {
		rnd->rnd_nhop = NULL;
		rnd->rnd_weight = 0;
	}

	return (rt);
}

/*
 * Runs exact prefix match based on dst/netmask from @info.
 * Assumes RIB lock is held.
 * Returns matched @rtentry if found or NULL.
 * If rtentry was found, saves nexthop / weight value into @rnd.
 */
struct rtentry *
lookup_prefix(struct rib_head *rnh, const struct rt_addrinfo *info,
    struct route_nhop_data *rnd)
{
	struct rtentry *rt;

	rt = lookup_prefix_bysa(rnh, info->rti_info[RTAX_DST],
	    info->rti_info[RTAX_NETMASK], rnd);

	return (rt);
}

/*
 * Adds route defined by @info into the kernel table specified by @fibnum and
 * sa_family in @info->rti_info[RTAX_DST].
 *
 * Returns 0 on success and fills in operation metadata into @rc.
 */
int
rib_add_route(uint32_t fibnum, struct rt_addrinfo *info,
    struct rib_cmd_info *rc)
{
	struct rib_head *rnh;
	int error;

	NET_EPOCH_ASSERT();

	rnh = get_rnh(fibnum, info);
	if (rnh == NULL)
		return (EAFNOSUPPORT);

	/*
	 * Check consistency between RTF_HOST flag and netmask
	 * existence.
	 */
	if (info->rti_flags & RTF_HOST)
		info->rti_info[RTAX_NETMASK] = NULL;
	else if (info->rti_info[RTAX_NETMASK] == NULL)
		return (EINVAL);

	bzero(rc, sizeof(struct rib_cmd_info));
	rc->rc_cmd = RTM_ADD;

	error = add_route(rnh, info, rc);
	if (error == 0)
		rib_notify(rnh, RIB_NOTIFY_DELAYED, rc);

	return (error);
}

/*
 * Creates rtentry and nexthop based on @info data.
 * Return 0 and fills in rtentry into @prt on success,
 * return errno otherwise.
 */
static int
create_rtentry(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rtentry **prt)
{
	struct sockaddr *dst, *ndst, *gateway, *netmask;
	struct rtentry *rt;
	struct nhop_object *nh;
	struct ifaddr *ifa;
	int error, flags;

	dst = info->rti_info[RTAX_DST];
	gateway = info->rti_info[RTAX_GATEWAY];
	netmask = info->rti_info[RTAX_NETMASK];
	flags = info->rti_flags;

	if ((flags & RTF_GATEWAY) && !gateway)
		return (EINVAL);
	if (dst && gateway && (dst->sa_family != gateway->sa_family) && 
	    (gateway->sa_family != AF_UNSPEC) && (gateway->sa_family != AF_LINK))
		return (EINVAL);

	if (dst->sa_len > sizeof(((struct rtentry *)NULL)->rt_dstb))
		return (EINVAL);

	if (info->rti_ifa == NULL) {
		error = rt_getifa_fib(info, rnh->rib_fibnum);
		if (error)
			return (error);
	} else {
		ifa_ref(info->rti_ifa);
	}

	error = nhop_create_from_info(rnh, info, &nh);
	if (error != 0) {
		ifa_free(info->rti_ifa);
		return (error);
	}

	rt = uma_zalloc(V_rtzone, M_NOWAIT | M_ZERO);
	if (rt == NULL) {
		ifa_free(info->rti_ifa);
		nhop_free(nh);
		return (ENOBUFS);
	}
	rt->rte_flags = (RTF_UP | flags) & RTE_RT_FLAG_MASK;
	rt->rt_nhop = nh;

	/* Fill in dst */
	memcpy(&rt->rt_dst, dst, dst->sa_len);
	rt_key(rt) = &rt->rt_dst;

	/*
	 * point to the (possibly newly malloc'd) dest address.
	 */
	ndst = (struct sockaddr *)rt_key(rt);

	/*
	 * make sure it contains the value we want (masked if needed).
	 */
	if (netmask) {
		rt_maskedcopy(dst, ndst, netmask);
	} else
		bcopy(dst, ndst, dst->sa_len);

	/*
	 * We use the ifa reference returned by rt_getifa_fib().
	 * This moved from below so that rnh->rnh_addaddr() can
	 * examine the ifa and  ifa->ifa_ifp if it so desires.
	 */
	ifa = info->rti_ifa;
	rt->rt_weight = get_info_weight(info, RT_DEFAULT_WEIGHT);
	rt_set_expire_info(rt, info);

	*prt = rt;
	return (0);
}

static int
add_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rib_cmd_info *rc)
{
	struct nhop_object *nh_orig;
	struct route_nhop_data rnd;
	struct nhop_object *nh;
	struct rtentry *rt, *rt_orig;
	int error;

	error = create_rtentry(rnh, info, &rt);
	if (error != 0)
		return (error);

	rnd.rnd_nhop = rt->rt_nhop;
	rnd.rnd_weight = rt->rt_weight;
	nh = rt->rt_nhop;

	RIB_WLOCK(rnh);
#ifdef RADIX_MPATH
	struct sockaddr *netmask;
	netmask = info->rti_info[RTAX_NETMASK];
	/* do not permit exactly the same dst/mask/gw pair */
	if (rt_mpath_capable(rnh) &&
		rt_mpath_conflict(rnh, rt, netmask)) {
		RIB_WUNLOCK(rnh);

		nhop_free(nh);
		uma_zfree(V_rtzone, rt);
		return (EEXIST);
	}
#endif
	error = add_route_nhop(rnh, rt, info, &rnd, rc);
	if (error == 0) {
		RIB_WUNLOCK(rnh);
		return (0);
	}

	/* addition failed. Lookup prefix in the rib to determine the cause */
	rt_orig = lookup_prefix(rnh, info, &rnd);
	if (rt_orig == NULL) {
		/* No prefix -> rnh_addaddr() failed to allocate memory */
		RIB_WUNLOCK(rnh);
		nhop_free(nh);
		uma_zfree(V_rtzone, rt);
		return (ENOMEM);
	}

	/* We have existing route in the RIB. */
	nh_orig = rnd.rnd_nhop;
	/* Check if new route has higher preference */
	if (can_override_nhop(info, nh_orig) > 0) {
		/* Update nexthop to the new route */
		change_route_nhop(rnh, rt_orig, info, &rnd, rc);
		RIB_WUNLOCK(rnh);
		uma_zfree(V_rtzone, rt);
		nhop_free(nh_orig);
		return (0);
	}

	RIB_WUNLOCK(rnh);

	/* Unable to add - another route with the same preference exists */
	error = EEXIST;

	nhop_free(nh);
	uma_zfree(V_rtzone, rt);

	return (error);
}

/*
 * Removes route defined by @info from the kernel table specified by @fibnum and
 * sa_family in @info->rti_info[RTAX_DST].
 *
 * Returns 0 on success and fills in operation metadata into @rc.
 */
int
rib_del_route(uint32_t fibnum, struct rt_addrinfo *info, struct rib_cmd_info *rc)
{
	struct rib_head *rnh;
	struct sockaddr *dst_orig, *netmask;
	struct sockaddr_storage mdst;
	int error;

	NET_EPOCH_ASSERT();

	rnh = get_rnh(fibnum, info);
	if (rnh == NULL)
		return (EAFNOSUPPORT);

	bzero(rc, sizeof(struct rib_cmd_info));
	rc->rc_cmd = RTM_DELETE;

	dst_orig = info->rti_info[RTAX_DST];
	netmask = info->rti_info[RTAX_NETMASK];

	if (netmask != NULL) {
		/* Ensure @dst is always properly masked */
		if (dst_orig->sa_len > sizeof(mdst))
			return (EINVAL);
		rt_maskedcopy(dst_orig, (struct sockaddr *)&mdst, netmask);
		info->rti_info[RTAX_DST] = (struct sockaddr *)&mdst;
	}
	error = del_route(rnh, info, rc);
	info->rti_info[RTAX_DST] = dst_orig;

	return (error);
}

/*
 * Conditionally unlinks rtentry matching data inside @info from @rnh.
 * Returns 0 on success with operation result stored in @rc.
 * On error, returns:
 * ESRCH - if prefix was not found,
 * EADDRINUSE - if trying to delete higher priority route.
 * ENOENT - if supplied filter function returned 0 (not matched).
 */
static int
rt_unlinkrte(struct rib_head *rnh, struct rt_addrinfo *info, struct rib_cmd_info *rc)
{
	struct rtentry *rt;
	struct nhop_object *nh;
	struct radix_node *rn;
	struct route_nhop_data rnd;
	int error;

	rt = lookup_prefix(rnh, info, &rnd);
	if (rt == NULL)
		return (ESRCH);

	nh = rt->rt_nhop;

	error = check_info_match_nhop(info, rt, nh);
	if (error != 0)
		return (error);

	if (can_override_nhop(info, nh) < 0)
		return (EADDRINUSE);

	/*
	 * Remove the item from the tree and return it.
	 * Complain if it is not there and do no more processing.
	 */
#ifdef RADIX_MPATH
	info->rti_info[RTAX_GATEWAY] = &nh->gw_sa;
	if (rt_mpath_capable(rnh)) {
		rn = rt_mpath_unlink(rnh, info, rt, &error);
		if (error != 0)
			return (error);
	} else
#endif
	rn = rnh->rnh_deladdr(info->rti_info[RTAX_DST],
	    info->rti_info[RTAX_NETMASK], &rnh->head);
	if (rn == NULL)
		return (ESRCH);

	if (rn->rn_flags & (RNF_ACTIVE | RNF_ROOT))
		panic ("rtrequest delete");

	rt = RNTORT(rn);
	rt->rte_flags &= ~RTF_UP;

	/* Finalize notification */
	rnh->rnh_gen++;
	rc->rc_cmd = RTM_DELETE;
	rc->rc_rt = rt;
	rc->rc_nh_old = rt->rt_nhop;
	rc->rc_nh_weight = rt->rt_weight;
	rib_notify(rnh, RIB_NOTIFY_IMMEDIATE, rc);

	return (0);
}

static int
del_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct rib_cmd_info *rc)
{
	int error;

	RIB_WLOCK(rnh);
	error = rt_unlinkrte(rnh, info, rc);
	RIB_WUNLOCK(rnh);
	if (error != 0)
		return (error);

	rib_notify(rnh, RIB_NOTIFY_DELAYED, rc);

	/*
	 * If the caller wants it, then it can have it,
	 * the entry will be deleted after the end of the current epoch.
	 */
	rtfree(rc->rc_rt);

	return (0);
}

int
rib_change_route(uint32_t fibnum, struct rt_addrinfo *info,
    struct rib_cmd_info *rc)
{
	RIB_RLOCK_TRACKER;
	struct route_nhop_data rnd_orig;
	struct rib_head *rnh;
	struct rtentry *rt;
	int error;

	NET_EPOCH_ASSERT();

	rnh = get_rnh(fibnum, info);
	if (rnh == NULL)
		return (EAFNOSUPPORT);

	bzero(rc, sizeof(struct rib_cmd_info));
	rc->rc_cmd = RTM_CHANGE;

	/* Check if updated gateway exists */
	if ((info->rti_flags & RTF_GATEWAY) &&
	    (info->rti_info[RTAX_GATEWAY] == NULL))
		return (EINVAL);

	/*
	 * route change is done in multiple steps, with dropping and
	 * reacquiring lock. In the situations with multiple processes
	 * changes the same route in can lead to the case when route
	 * is changed between the steps. Address it by retrying the operation
	 * multiple times before failing.
	 */

	RIB_RLOCK(rnh);
	rt = (struct rtentry *)rnh->rnh_lookup(info->rti_info[RTAX_DST],
	    info->rti_info[RTAX_NETMASK], &rnh->head);

	if (rt == NULL) {
		RIB_RUNLOCK(rnh);
		return (ESRCH);
	}

#ifdef RADIX_MPATH
	/*
	 * If we got multipath routes,
	 * we require users to specify a matching RTAX_GATEWAY.
	 */
	if (rt_mpath_capable(rnh)) {
		rt = rt_mpath_matchgate(rt, info->rti_info[RTAX_GATEWAY]);
		if (rt == NULL) {
			RIB_RUNLOCK(rnh);
			return (ESRCH);
		}
	}
#endif
	rnd_orig.rnd_nhop = rt->rt_nhop;
	rnd_orig.rnd_weight = rt->rt_weight;

	RIB_RUNLOCK(rnh);

	for (int i = 0; i < RIB_MAX_RETRIES; i++) {
		error = change_route(rnh, info, &rnd_orig, rc);
		if (error != EAGAIN)
			break;
	}

	return (error);
}

static int
change_route(struct rib_head *rnh, struct rt_addrinfo *info,
    struct route_nhop_data *rnd_orig, struct rib_cmd_info *rc)
{
	int error = 0;
	int free_ifa = 0;
	struct nhop_object *nh, *nh_orig;
	struct route_nhop_data rnd_new;

	nh = NULL;
	nh_orig = rnd_orig->rnd_nhop;
	if (nh_orig == NULL)
		return (ESRCH);

	/*
	 * New gateway could require new ifaddr, ifp;
	 * flags may also be different; ifp may be specified
	 * by ll sockaddr when protocol address is ambiguous
	 */
	if (((nh_orig->nh_flags & NHF_GATEWAY) &&
	    info->rti_info[RTAX_GATEWAY] != NULL) ||
	    info->rti_info[RTAX_IFP] != NULL ||
	    (info->rti_info[RTAX_IFA] != NULL &&
	     !sa_equal(info->rti_info[RTAX_IFA], nh_orig->nh_ifa->ifa_addr))) {
		error = rt_getifa_fib(info, rnh->rib_fibnum);
		if (info->rti_ifa != NULL)
			free_ifa = 1;

		if (error != 0) {
			if (free_ifa) {
				ifa_free(info->rti_ifa);
				info->rti_ifa = NULL;
			}

			return (error);
		}
	}

	error = nhop_create_from_nhop(rnh, nh_orig, info, &nh);
	if (free_ifa) {
		ifa_free(info->rti_ifa);
		info->rti_ifa = NULL;
	}
	if (error != 0)
		return (error);

	rnd_new.rnd_nhop = nh;
	if (info->rti_mflags & RTV_WEIGHT)
		rnd_new.rnd_weight = info->rti_rmx->rmx_weight;
	else
		rnd_new.rnd_weight = rnd_orig->rnd_weight;

	error = change_route_conditional(rnh, NULL, info, rnd_orig, &rnd_new, rc);

	return (error);
}

/*
 * Insert @rt with nhop data from @rnd_new to @rnh.
 * Returns 0 on success and stores operation results in @rc.
 */
static int
add_route_nhop(struct rib_head *rnh, struct rtentry *rt,
    struct rt_addrinfo *info, struct route_nhop_data *rnd,
    struct rib_cmd_info *rc)
{
	struct sockaddr *ndst, *netmask;
	struct radix_node *rn;
	int error = 0;

	RIB_WLOCK_ASSERT(rnh);

	ndst = (struct sockaddr *)rt_key(rt);
	netmask = info->rti_info[RTAX_NETMASK];

	rt->rt_nhop = rnd->rnd_nhop;
	rt->rt_weight = rnd->rnd_weight;
	rn = rnh->rnh_addaddr(ndst, netmask, &rnh->head, rt->rt_nodes);

	if (rn != NULL) {
		if (rt->rt_expire > 0)
			tmproutes_update(rnh, rt);

		/* Finalize notification */
		rnh->rnh_gen++;

		rc->rc_cmd = RTM_ADD;
		rc->rc_rt = rt;
		rc->rc_nh_old = NULL;
		rc->rc_nh_new = rnd->rnd_nhop;
		rc->rc_nh_weight = rnd->rnd_weight;

		rib_notify(rnh, RIB_NOTIFY_IMMEDIATE, rc);
	} else {
		/* Existing route or memory allocation failure */
		error = EEXIST;
	}

	return (error);
}

/*
 * Switch @rt nhop/weigh to the ones specified in @rnd.
 *  Conditionally set rt_expire if set in @info.
 * Returns 0 on success.
 */
static int
change_route_nhop(struct rib_head *rnh, struct rtentry *rt,
    struct rt_addrinfo *info, struct route_nhop_data *rnd,
    struct rib_cmd_info *rc)
{
	struct nhop_object *nh_orig;

	RIB_WLOCK_ASSERT(rnh);

	nh_orig = rt->rt_nhop;

	if (rnd->rnd_nhop != NULL) {
		/* Changing expiration & nexthop & weight to a new one */
		rt_set_expire_info(rt, info);
		rt->rt_nhop = rnd->rnd_nhop;
		rt->rt_weight = rnd->rnd_weight;
		if (rt->rt_expire > 0)
			tmproutes_update(rnh, rt);
	} else {
		/* Route deletion requested. */
		struct sockaddr *ndst, *netmask;
		struct radix_node *rn;

		ndst = (struct sockaddr *)rt_key(rt);
		netmask = info->rti_info[RTAX_NETMASK];
		rn = rnh->rnh_deladdr(ndst, netmask, &rnh->head);
		if (rn == NULL)
			return (ESRCH);
	}

	/* Finalize notification */
	rnh->rnh_gen++;

	rc->rc_cmd = (rnd->rnd_nhop != NULL) ? RTM_CHANGE : RTM_DELETE;
	rc->rc_rt = rt;
	rc->rc_nh_old = nh_orig;
	rc->rc_nh_new = rnd->rnd_nhop;
	rc->rc_nh_weight = rnd->rnd_weight;

	rib_notify(rnh, RIB_NOTIFY_IMMEDIATE, rc);

	return (0);
}

/*
 * Conditionally update route nhop/weight IFF data in @nhd_orig is
 *  consistent with the current route data.
 * Nexthop in @nhd_new is consumed.
 */
int
change_route_conditional(struct rib_head *rnh, struct rtentry *rt,
    struct rt_addrinfo *info, struct route_nhop_data *rnd_orig,
    struct route_nhop_data *rnd_new, struct rib_cmd_info *rc)
{
	struct rtentry *rt_new;
	int error = 0;

	RIB_WLOCK(rnh);

	rt_new = (struct rtentry *)rnh->rnh_lookup(info->rti_info[RTAX_DST],
	    info->rti_info[RTAX_NETMASK], &rnh->head);

	if (rt_new == NULL) {
		if (rnd_orig->rnd_nhop == NULL)
			error = add_route_nhop(rnh, rt, info, rnd_new, rc);
		else {
			/*
			 * Prefix does not exist, which was not our assumption.
			 * Update @rnd_orig with the new data and return
			 */
			rnd_orig->rnd_nhop = NULL;
			rnd_orig->rnd_weight = 0;
			error = EAGAIN;
		}
	} else {
		/* Prefix exists, try to update */
		if (rnd_orig->rnd_nhop == rt_new->rt_nhop) {
			/*
			 * Nhop/mpath group hasn't changed. Flip
			 * to the new precalculated one and return
			 */
			error = change_route_nhop(rnh, rt_new, info, rnd_new, rc);
		} else {
			/* Update and retry */
			rnd_orig->rnd_nhop = rt_new->rt_nhop;
			rnd_orig->rnd_weight = rt_new->rt_weight;
			error = EAGAIN;
		}
	}

	RIB_WUNLOCK(rnh);

	if (error == 0) {
		rib_notify(rnh, RIB_NOTIFY_DELAYED, rc);

		if (rnd_orig->rnd_nhop != NULL)
			nhop_free_any(rnd_orig->rnd_nhop);

	} else {
		if (rnd_new->rnd_nhop != NULL)
			nhop_free_any(rnd_new->rnd_nhop);
	}

	return (error);
}

/*
 * Performs modification of routing table specificed by @action.
 * Table is specified by @fibnum and sa_family in @info->rti_info[RTAX_DST].
 * Needs to be run in network epoch.
 *
 * Returns 0 on success and fills in @rc with action result.
 */
int
rib_action(uint32_t fibnum, int action, struct rt_addrinfo *info,
    struct rib_cmd_info *rc)
{
	int error;

	switch (action) {
	case RTM_ADD:
		error = rib_add_route(fibnum, info, rc);
		break;
	case RTM_DELETE:
		error = rib_del_route(fibnum, info, rc);
		break;
	case RTM_CHANGE:
		error = rib_change_route(fibnum, info, rc);
		break;
	default:
		error = ENOTSUP;
	}

	return (error);
}

struct rt_delinfo
{
	struct rt_addrinfo info;
	struct rib_head *rnh;
	struct rtentry *head;
	struct rib_cmd_info rc;
};

/*
 * Conditionally unlinks @rn from radix tree based
 * on info data passed in @arg.
 */
static int
rt_checkdelroute(struct radix_node *rn, void *arg)
{
	struct rt_delinfo *di;
	struct rt_addrinfo *info;
	struct rtentry *rt;
	int error;

	di = (struct rt_delinfo *)arg;
	rt = (struct rtentry *)rn;
	info = &di->info;

	info->rti_info[RTAX_DST] = rt_key(rt);
	info->rti_info[RTAX_NETMASK] = rt_mask(rt);
	info->rti_info[RTAX_GATEWAY] = &rt->rt_nhop->gw_sa;

	error = rt_unlinkrte(di->rnh, info, &di->rc);

	/*
	 * Add deleted rtentries to the list to GC them
	 *  after dropping the lock.
	 *
	 * XXX: Delayed notifications not implemented
	 *  for nexthop updates.
	 */
	if (error == 0) {
		/* Add to the list and return */
		rt->rt_chain = di->head;
		di->head = rt;
	}

	return (0);
}

/*
 * Iterates over a routing table specified by @fibnum and @family and
 *  deletes elements marked by @filter_f.
 * @fibnum: rtable id
 * @family: AF_ address family
 * @filter_f: function returning non-zero value for items to delete
 * @arg: data to pass to the @filter_f function
 * @report: true if rtsock notification is needed.
 */
void
rib_walk_del(u_int fibnum, int family, rt_filter_f_t *filter_f, void *arg, bool report)
{
	struct rib_head *rnh;
	struct rt_delinfo di;
	struct rtentry *rt;
	struct epoch_tracker et;

	rnh = rt_tables_get_rnh(fibnum, family);
	if (rnh == NULL)
		return;

	bzero(&di, sizeof(di));
	di.info.rti_filter = filter_f;
	di.info.rti_filterdata = arg;
	di.rnh = rnh;
	di.rc.rc_cmd = RTM_DELETE;

	NET_EPOCH_ENTER(et);

	RIB_WLOCK(rnh);
	rnh->rnh_walktree(&rnh->head, rt_checkdelroute, &di);
	RIB_WUNLOCK(rnh);

	/* We might have something to reclaim. */
	bzero(&di.rc, sizeof(di.rc));
	di.rc.rc_cmd = RTM_DELETE;
	while (di.head != NULL) {
		rt = di.head;
		di.head = rt->rt_chain;
		rt->rt_chain = NULL;

		di.rc.rc_rt = rt;
		di.rc.rc_nh_old = rt->rt_nhop;
		rib_notify(rnh, RIB_NOTIFY_DELAYED, &di.rc);

		/* TODO std rt -> rt_addrinfo export */
		di.info.rti_info[RTAX_DST] = rt_key(rt);
		di.info.rti_info[RTAX_NETMASK] = rt_mask(rt);

		if (report)
			rt_routemsg(RTM_DELETE, rt, rt->rt_nhop->nh_ifp, 0,
			    fibnum);
		rtfree(rt);
	}

	NET_EPOCH_EXIT(et);
}

static void
rib_notify(struct rib_head *rnh, enum rib_subscription_type type,
    struct rib_cmd_info *rc)
{
	struct rib_subscription *rs;

	CK_STAILQ_FOREACH(rs, &rnh->rnh_subscribers, next) {
		if (rs->type == type)
			rs->func(rnh, rc, rs->arg);
	}
}

static struct rib_subscription *
allocate_subscription(rib_subscription_cb_t *f, void *arg,
    enum rib_subscription_type type, bool waitok)
{
	struct rib_subscription *rs;
	int flags = M_ZERO | (waitok ? M_WAITOK : 0);

	rs = malloc(sizeof(struct rib_subscription), M_RTABLE, flags);
	if (rs == NULL)
		return (NULL);

	rs->func = f;
	rs->arg = arg;
	rs->type = type;

	return (rs);
}

/*
 * Subscribe for the changes in the routing table specified by @fibnum and
 *  @family.
 *
 * Returns pointer to the subscription structure on success.
 */
struct rib_subscription *
rib_subscribe(uint32_t fibnum, int family, rib_subscription_cb_t *f, void *arg,
    enum rib_subscription_type type, bool waitok)
{
	struct rib_head *rnh;
	struct rib_subscription *rs;
	struct epoch_tracker et;

	if ((rs = allocate_subscription(f, arg, type, waitok)) == NULL)
		return (NULL);

	NET_EPOCH_ENTER(et);
	KASSERT((fibnum < rt_numfibs), ("%s: bad fibnum", __func__));
	rnh = rt_tables_get_rnh(fibnum, family);

	RIB_WLOCK(rnh);
	CK_STAILQ_INSERT_TAIL(&rnh->rnh_subscribers, rs, next);
	RIB_WUNLOCK(rnh);
	NET_EPOCH_EXIT(et);

	return (rs);
}

struct rib_subscription *
rib_subscribe_internal(struct rib_head *rnh, rib_subscription_cb_t *f, void *arg,
    enum rib_subscription_type type, bool waitok)
{
	struct rib_subscription *rs;
	struct epoch_tracker et;

	if ((rs = allocate_subscription(f, arg, type, waitok)) == NULL)
		return (NULL);

	NET_EPOCH_ENTER(et);
	RIB_WLOCK(rnh);
	CK_STAILQ_INSERT_TAIL(&rnh->rnh_subscribers, rs, next);
	RIB_WUNLOCK(rnh);
	NET_EPOCH_EXIT(et);

	return (rs);
}

/*
 * Remove rtable subscription @rs from the table specified by @fibnum
 *  and @family.
 * Needs to be run in network epoch.
 *
 * Returns 0 on success.
 */
int
rib_unsibscribe(uint32_t fibnum, int family, struct rib_subscription *rs)
{
	struct rib_head *rnh;

	NET_EPOCH_ASSERT();
	KASSERT((fibnum < rt_numfibs), ("%s: bad fibnum", __func__));
	rnh = rt_tables_get_rnh(fibnum, family);

	if (rnh == NULL)
		return (ENOENT);

	RIB_WLOCK(rnh);
	CK_STAILQ_REMOVE(&rnh->rnh_subscribers, rs, rib_subscription, next);
	RIB_WUNLOCK(rnh);

	epoch_call(net_epoch_preempt, destroy_subscription_epoch,
	    &rs->epoch_ctx);

	return (0);
}

/*
 * Epoch callback indicating subscription is safe to destroy
 */
static void
destroy_subscription_epoch(epoch_context_t ctx)
{
	struct rib_subscription *rs;

	rs = __containerof(ctx, struct rib_subscription, epoch_ctx);

	free(rs, M_RTABLE);
}

void
rib_init_subscriptions(struct rib_head *rnh)
{

	CK_STAILQ_INIT(&rnh->rnh_subscribers);
}

void
rib_destroy_subscriptions(struct rib_head *rnh)
{
	struct rib_subscription *rs;
	struct epoch_tracker et;

	NET_EPOCH_ENTER(et);
	RIB_WLOCK(rnh);
	while ((rs = CK_STAILQ_FIRST(&rnh->rnh_subscribers)) != NULL) {
		CK_STAILQ_REMOVE_HEAD(&rnh->rnh_subscribers, next);
		epoch_call(net_epoch_preempt, destroy_subscription_epoch,
		    &rs->epoch_ctx);
	}
	RIB_WUNLOCK(rnh);
	NET_EPOCH_EXIT(et);
}
