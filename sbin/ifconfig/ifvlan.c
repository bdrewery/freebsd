/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1999 Bill Paul <wpaul@ctr.columbia.edu>
 * Copyright (c) 2012 ADARA Networks, Inc.
 * All rights reserved.
  *
 * Portions of this software were developed by Robert N. M. Watson under
 * contract to ADARA Networks, Inc.
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
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <stdlib.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <net/route.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "ifconfig.h"

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif

#define	NOTAG	((u_short) -1)

static const char proto_8021Q[]  = "802.1q";
static const char proto_8021ad[] = "802.1ad";

static 	struct vlanreq params = {
	.vlr_tag	= NOTAG,
	.vlr_proto	= ETHERTYPE_VLAN,
};

static int
getvlan(int s, struct ifreq *ifr, struct vlanreq *vreq)
{
	bzero((char *)vreq, sizeof(*vreq));
	ifr->ifr_data = (caddr_t)vreq;

	return ioctl(s, SIOCGETVLAN, (caddr_t)ifr);
}

static void
vlan_status(int s)
{
	struct vlanreq		vreq;

	if (getvlan(s, &ifr, &vreq) == -1)
		return;
	printf("\tvlan: %d", vreq.vlr_tag);
	printf(" vlanproto: ");
	switch (vreq.vlr_proto) {
		case ETHERTYPE_VLAN:
			printf(proto_8021Q);
			break;
		case ETHERTYPE_QINQ:
			printf(proto_8021ad);
			break;
		default:
			printf("0x%04x", vreq.vlr_proto);
	}
	if (ioctl(s, SIOCGVLANPCP, (caddr_t)&ifr) != -1)
		printf(" vlanpcp: %u", ifr.ifr_vlan_pcp);
	printf(" parent interface: %s", vreq.vlr_parent[0] == '\0' ?
	    "<none>" : vreq.vlr_parent);
	printf("\n");
}

static int
vlan_match_ethervid(const char *name)
{
	return (strchr(name, '.') != NULL);
}

static void
vlan_parse_ethervid(const char *name)
{
	char ifname[IFNAMSIZ];
	char *cp;
	int vid;

	strlcpy(ifname, name, IFNAMSIZ);
	if ((cp = strrchr(ifname, '.')) == NULL)
		return;
	/*
	 * Don't mix vlan/vlandev parameters with dot notation.
	 */
	if (params.vlr_tag != NOTAG || params.vlr_parent[0] != '\0')
		errx(1, "ambiguous vlan specification");
	/*
	 * Derive params from interface name: "parent.vid".
	 */
	*cp++ = '\0';
	if ((*cp < '1') || (*cp > '9'))
		errx(1, "invalid vlan tag");

	vid = *cp++ - '0';
	while ((*cp >= '0') && (*cp <= '9'))
		vid = (vid * 10) + (*cp++ - '0');
	if ((*cp != '\0') || (vid & ~0xFFF))
		errx(1, "invalid vlan tag");

	strlcpy(params.vlr_parent, ifname, IFNAMSIZ);
	params.vlr_tag = (vid & 0xFFF);
}

static void
vlan_create(int s, struct ifreq *ifr)
{
	vlan_parse_ethervid(ifr->ifr_name);

	if (params.vlr_tag != NOTAG || params.vlr_parent[0] != '\0') {
		/*
		 * One or both parameters were specified, make sure both.
		 */
		if (params.vlr_tag == NOTAG)
			errx(1, "must specify a tag for vlan create");
		if (params.vlr_parent[0] == '\0')
			errx(1, "must specify a parent device for vlan create");
		ifr->ifr_data = (caddr_t) &params;
	}
	if (ioctl(s, SIOCIFCREATE2, ifr) < 0)
		err(1, "SIOCIFCREATE2");
}

static void
vlan_cb(int s, void *arg)
{
	if ((params.vlr_tag != NOTAG) ^ (params.vlr_parent[0] != '\0'))
		errx(1, "both vlan and vlandev must be specified");
}

static void
vlan_set(int s, struct ifreq *ifr)
{
	if (params.vlr_tag != NOTAG && params.vlr_parent[0] != '\0') {
		ifr->ifr_data = (caddr_t) &params;
		if (ioctl(s, SIOCSETVLAN, (caddr_t)ifr) == -1)
			err(1, "SIOCSETVLAN");
	}
}

static
DECL_CMD_FUNC(setvlantag, val, d)
{
	struct vlanreq vreq;
	u_long ul;
	char *endp;

	ul = strtoul(val, &endp, 0);
	if (*endp != '\0')
		errx(1, "invalid value for vlan");
	params.vlr_tag = ul;
	/* check if the value can be represented in vlr_tag */
	if (params.vlr_tag != ul)
		errx(1, "value for vlan out of range");

	if (getvlan(s, &ifr, &vreq) != -1)
		vlan_set(s, &ifr);
}

static
DECL_CMD_FUNC(setvlandev, val, d)
{
	struct vlanreq vreq;

	strlcpy(params.vlr_parent, val, sizeof(params.vlr_parent));

	if (getvlan(s, &ifr, &vreq) != -1)
		vlan_set(s, &ifr);
}

static
DECL_CMD_FUNC(setvlanproto, val, d)
{
	struct vlanreq vreq;

	if (strncasecmp(proto_8021Q, val,
	    strlen(proto_8021Q)) == 0) {
		params.vlr_proto = ETHERTYPE_VLAN;
	} else if (strncasecmp(proto_8021ad, val,
	           strlen(proto_8021ad)) == 0) {
		params.vlr_proto = ETHERTYPE_QINQ;
	} else
		errx(1, "invalid value for vlanproto");

	if (getvlan(s, &ifr, &vreq) != -1)
		vlan_set(s, &ifr);
}

static
DECL_CMD_FUNC(setvlanpcp, val, d)
{
	u_long ul;
	char *endp;

	ul = strtoul(val, &endp, 0);
	if (*endp != '\0')
		errx(1, "invalid value for vlanpcp");
	if (ul > 7)
		errx(1, "value for vlanpcp out of range");
	ifr.ifr_vlan_pcp = ul;
	if (ioctl(s, SIOCSVLANPCP, (caddr_t)&ifr) == -1)
		err(1, "SIOCSVLANPCP");
}

static
DECL_CMD_FUNC(unsetvlandev, val, d)
{
	struct vlanreq		vreq;

	bzero((char *)&vreq, sizeof(struct vlanreq));
	ifr.ifr_data = (caddr_t)&vreq;

	if (ioctl(s, SIOCGETVLAN, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETVLAN");

	bzero((char *)&vreq.vlr_parent, sizeof(vreq.vlr_parent));
	vreq.vlr_tag = 0;

	if (ioctl(s, SIOCSETVLAN, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETVLAN");
}

static struct cmd vlan_cmds[] = {
	DEF_CLONE_CMD_ARG("vlan",			setvlantag),
	DEF_CLONE_CMD_ARG("vlandev",			setvlandev),
	DEF_CLONE_CMD_ARG("vlanproto",			setvlanproto),
	DEF_CMD_ARG("vlanpcp",				setvlanpcp),
	/* NB: non-clone cmds */
	DEF_CMD_ARG("vlan",				setvlantag),
	DEF_CMD_ARG("vlandev",				setvlandev),
	DEF_CMD_ARG("vlanproto",			setvlanproto),
	/* XXX For compatibility.  Should become DEF_CMD() some day. */
	DEF_CMD_OPTARG("-vlandev",			unsetvlandev),
	DEF_CMD("vlanmtu",	IFCAP_VLAN_MTU,		setifcap),
	DEF_CMD("-vlanmtu",	-IFCAP_VLAN_MTU,	setifcap),
	DEF_CMD("vlanhwtag",	IFCAP_VLAN_HWTAGGING,	setifcap),
	DEF_CMD("-vlanhwtag",	-IFCAP_VLAN_HWTAGGING,	setifcap),
	DEF_CMD("vlanhwfilter",	IFCAP_VLAN_HWFILTER,	setifcap),
	DEF_CMD("-vlanhwfilter", -IFCAP_VLAN_HWFILTER,	setifcap),
	DEF_CMD("-vlanhwtso",	-IFCAP_VLAN_HWTSO,	setifcap),
	DEF_CMD("vlanhwtso",	IFCAP_VLAN_HWTSO,	setifcap),
	DEF_CMD("vlanhwcsum",	IFCAP_VLAN_HWCSUM,	setifcap),
	DEF_CMD("-vlanhwcsum",	-IFCAP_VLAN_HWCSUM,	setifcap),
};
static struct afswtch af_vlan = {
	.af_name	= "af_vlan",
	.af_af		= AF_UNSPEC,
	.af_other_status = vlan_status,
};

static __constructor void
vlan_ctor(void)
{
	size_t i;

	for (i = 0; i < nitems(vlan_cmds);  i++)
		cmd_register(&vlan_cmds[i]);
	af_register(&af_vlan);
	callback_register(vlan_cb, NULL);
	clone_setdefcallback_prefix("vlan", vlan_create);
	clone_setdefcallback_filter(vlan_match_ethervid, vlan_create);
}
