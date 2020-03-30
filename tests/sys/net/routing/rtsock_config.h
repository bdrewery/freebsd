/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2019 Alexander V. Chernikov
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
 *
 * $FreeBSD$
 */

#ifndef _NET_ROUTING_RTSOCK_CONFIG_H_
#define _NET_ROUTING_RTSOCK_CONFIG_H_

#include "params.h"

struct rtsock_test_config {
	int ifindex;
	char net4_str[INET_ADDRSTRLEN];
	char addr4_str[INET_ADDRSTRLEN];
	char net6_str[INET6_ADDRSTRLEN];
	char addr6_str[INET6_ADDRSTRLEN];
	struct sockaddr_in net4;
	struct sockaddr_in mask4;
	struct sockaddr_in addr4;
	struct sockaddr_in6 net6;
	struct sockaddr_in6 mask6;
	struct sockaddr_in6 addr6;
	int plen4;
	int plen6;
	char *remote_lladdr;
	char *ifname;
	bool autocreated_interface;
	int rtsock_fd;
};

struct rtsock_test_config *
config_setup_base(const atf_tc_t *tc)
{
	struct rtsock_test_config *c;

	c = calloc(1, sizeof(struct rtsock_test_config));
	c->rtsock_fd = -1;

	return c;
}

struct rtsock_test_config *
config_setup(const atf_tc_t *tc)
{
	struct rtsock_test_config *c;
	char buf[64], *s;
	const char *key;
	int mask;

	c = config_setup_base(tc);

	key = atf_tc_get_config_var_wd(tc, "rtsock.v4prefix", "192.0.2.0/24");
	strlcpy(buf, key, sizeof(buf));
	if ((s = strchr(buf, '/')) == NULL)
		return (NULL);
	*s++ = '\0';
	mask = strtol(s, NULL, 10);
	if (mask < 0 || mask > 32)
		return (NULL);
	c->plen4 = mask;
	inet_pton(AF_INET, buf, &c->net4.sin_addr);

	c->net4.sin_len = sizeof(struct sockaddr_in);
	c->net4.sin_family = AF_INET;
	c->addr4.sin_len = sizeof(struct sockaddr_in);
	c->addr4.sin_family = AF_INET;

	sa_fill_mask4(&c->mask4, c->plen4);

	/* Fill in interface IPv4 address. Assume the first address in net */
	c->addr4.sin_addr.s_addr = htonl(ntohl(c->net4.sin_addr.s_addr) + 1);
	inet_ntop(AF_INET, &c->net4.sin_addr, c->net4_str, INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &c->addr4.sin_addr, c->addr4_str, INET_ADDRSTRLEN);

	key = atf_tc_get_config_var_wd(tc, "rtsock.v6prefix", "2001:DB8::/32");
	strlcpy(buf, key, sizeof(buf));
	if ((s = strchr(buf, '/')) == NULL)
		return (NULL);
	*s++ = '\0';
	mask = strtol(s, NULL, 10);
	if (mask < 0 || mask > 128)
		return (NULL);
	c->plen6 = mask;

	inet_pton(AF_INET6, buf, &c->net6.sin6_addr);

	c->net6.sin6_len = sizeof(struct sockaddr_in6);
	c->net6.sin6_family = AF_INET6;
	c->addr6.sin6_len = sizeof(struct sockaddr_in6);
	c->addr6.sin6_family = AF_INET6;

	sa_fill_mask6(&c->mask6, c->plen6);

	/* Fill in interface IPv6 address. Assume the first address in net */
	memcpy(&c->addr6.sin6_addr, &c->net6.sin6_addr, sizeof(struct in6_addr));
#define _s6_addr32 __u6_addr.__u6_addr32
	c->addr6.sin6_addr._s6_addr32[3] = htonl(ntohl(c->net6.sin6_addr._s6_addr32[3]) + 1);
#undef _s6_addr32
	inet_ntop(AF_INET6, &c->net6.sin6_addr, c->net6_str, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, &c->addr6.sin6_addr, c->addr6_str, INET6_ADDRSTRLEN);

	c->ifname = strdup("epair");
	c->autocreated_interface = true;

	if (c->autocreated_interface && (if_nametoindex(c->ifname) == 0))
       	{
		/* create our own interface */
		char new_ifname[IFNAMSIZ];
		strlcpy(new_ifname, c->ifname, sizeof(new_ifname));
		int ret = iface_create_cloned(new_ifname);
		ATF_REQUIRE_MSG(ret != 0, "%s interface creation failed: %s", new_ifname,
		    strerror(errno));
		c->ifname = strdup(new_ifname);
		file_append_line(IFACES_FNAME, new_ifname);
		if (strstr(new_ifname, "epair") == new_ifname) {
			/* call returned epairXXXa, need to add epairXXXb */
			new_ifname[strlen(new_ifname) - 1] = 'b';
			file_append_line(IFACES_FNAME, new_ifname);
		}
	}
	c->ifindex = if_nametoindex(c->ifname);
	ATF_REQUIRE_MSG(c->ifindex != 0, "inteface %s not found", c->ifname);

	c->remote_lladdr = strdup(atf_tc_get_config_var_wd(tc,
	    "rtsock.remote_lladdr", "00:00:5E:00:53:42"));

	return (c);
}

void
config_generic_cleanup(const atf_tc_t *tc)
{
	const char *srcdir = atf_tc_get_config_var(tc, "srcdir");
	char cmd[512];
	int ret;

	/* XXX: sleep 100ms to avoid epair qflush panic */
	usleep(1000 * 100);
	snprintf(cmd, sizeof(cmd), "%s/generic_cleanup.sh", srcdir);
	ret = system(cmd);
	if (ret != 0)
		RLOG("'%s' failed, error %d", cmd, ret);
}

void
config_describe_root_test(atf_tc_t *tc, char *test_descr)
{

	atf_tc_set_md_var(tc, "descr", test_descr);
	// Adding/deleting prefix requires root privileges
	atf_tc_set_md_var(tc, "require.user", "root");
}

#endif
