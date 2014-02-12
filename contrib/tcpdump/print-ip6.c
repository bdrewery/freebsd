/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD$
 */

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/print-ip6.c,v 1.52 2007-09-21 07:05:33 hannes Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef INET6

#include <tcpdump-stdinc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netdissect.h"
#include "interface.h"
#include "addrtoname.h"
#include "extract.h"

#include "ip6.h"
#include "ipproto.h"

/*
 * Compute a V6-style checksum by building a pseudoheader.
 */
int
nextproto6_cksum(__capability const struct ip6_hdr *ip6,
		 __capability const u_int8_t *data,
		 u_int len, u_int next_proto)
{
        struct {
                struct in6_addr ph_src;
                struct in6_addr ph_dst;
                u_int32_t       ph_len;
                u_int8_t        ph_zero[3];
                u_int8_t        ph_nxt;
        } ph;
        struct cksum_vec vec[2];

        /* pseudo-header */
        memset(&ph, 0, sizeof(ph));
        ph.ph_src = ip6->ip6_src;
        ph.ph_dst = ip6->ip6_dst;
        ph.ph_len = htonl(len);
        ph.ph_nxt = next_proto;

        vec[0].ptr = cheri_ptr(&ph, sizeof(ph));
        vec[0].len = sizeof(ph);
        vec[1].ptr = data;
        vec[1].len = len;

        return in_cksum(vec, 2);
}

/*
 * print an IP6 datagram.
 */
void
ip6_print(netdissect_options *ndo, packetbody_t bp, u_int length)
{
	__capability const struct ip6_hdr *ip6;
	register int advance;
	u_int len;
	packetbody_t cp;
	register u_int payload_len;
	int nh;
	int fragmented = 0;
	u_int flow;

	ip6 = (__capability const struct ip6_hdr *)bp;

	TCHECK(*ip6);
	if (length < sizeof (struct ip6_hdr)) {
		(void)ND_PRINT((ndo, "truncated-ip6 %u", length));
		return;
	}

        if (!ndo->ndo_eflag)
            ND_PRINT((ndo, "IP6 "));

	payload_len = EXTRACT_16BITS(&ip6->ip6_plen);
	len = payload_len + sizeof(struct ip6_hdr);
	if (length < len)
		(void)ND_PRINT((ndo, "truncated-ip6 - %u bytes missing!",
			len - length));

        if (ndo->ndo_vflag) {
            flow = EXTRACT_32BITS(&ip6->ip6_flow);
            ND_PRINT((ndo, "("));
#if 0
            /* rfc1883 */
            if (flow & 0x0f000000)
		(void)ND_PRINT((ndo, "pri 0x%02x, ", (flow & 0x0f000000) >> 24));
            if (flow & 0x00ffffff)
		(void)ND_PRINT((ndo, "flowlabel 0x%06x, ", flow & 0x00ffffff));
#else
            /* RFC 2460 */
            if (flow & 0x0ff00000)
		(void)ND_PRINT((ndo, "class 0x%02x, ", (flow & 0x0ff00000) >> 20));
            if (flow & 0x000fffff)
		(void)ND_PRINT((ndo, "flowlabel 0x%05x, ", flow & 0x000fffff));
#endif

            (void)ND_PRINT((ndo, "hlim %u, next-header %s (%u) payload length: %u) ",
                         ip6->ip6_hlim,
                         tok2str(ipproto_values,"unknown",ip6->ip6_nxt),
                         ip6->ip6_nxt,
                         payload_len));
        }

	/*
	 * Cut off the snapshot length to the end of the IP payload.
	 */
	/* XXX-BD: should create new truncated capability on CHERI */
 	if (PACKET_REMAINING(bp) > len)
		ndo->ndo_snapend = bp + len;

	cp = (packetbody_t)ip6;
	advance = sizeof(struct ip6_hdr);
	nh = ip6->ip6_nxt;
	/* XXX-BD: previously could run off the end */
	while (ND_TTEST2(cp, advance)) {
		cp += advance;
		len -= advance;

		if (cp == (packetbody_t)(ip6 + 1) &&
		    nh != IPPROTO_TCP && nh != IPPROTO_UDP &&
		    nh != IPPROTO_DCCP && nh != IPPROTO_SCTP) {
			(void)ND_PRINT((ndo, "%s > %s: ", ip6addr_string(&ip6->ip6_src),
				     ip6addr_string(&ip6->ip6_dst)));
		}

		switch (nh) {
		case IPPROTO_HOPOPTS:
			advance = hbhopt_print(cp);
			nh = *cp;
			break;
		case IPPROTO_DSTOPTS:
			advance = dstopt_print(cp);
			nh = *cp;
			break;
		case IPPROTO_FRAGMENT:
			advance = frag6_print(cp, (packetbody_t)ip6);
			if (!ND_TTEST2(cp, advance))
				return;
			nh = *cp;
			fragmented = 1;
			break;

		case IPPROTO_MOBILITY_OLD:
		case IPPROTO_MOBILITY:
			/*
			 * XXX - we don't use "advance"; the current
			 * "Mobility Support in IPv6" draft
			 * (draft-ietf-mobileip-ipv6-24) says that
			 * the next header field in a mobility header
			 * should be IPPROTO_NONE, but speaks of
			 * the possiblity of a future extension in
			 * which payload can be piggybacked atop a
			 * mobility header.
			 */
			advance = mobility_print(cp, (packetbody_t)ip6);
			nh = *cp;
			return;
		case IPPROTO_ROUTING:
			advance = rt6_print(cp, (packetbody_t)ip6);
			nh = *cp;
			break;
		case IPPROTO_SCTP:
			sctp_print(cp, (packetbody_t)ip6, len);
			return;
		case IPPROTO_DCCP:
			dccp_print(cp, (packetbody_t)ip6, len);
			return;
		case IPPROTO_TCP:
			tcp_print(cp, len, (packetbody_t)ip6, fragmented);
			return;
		case IPPROTO_UDP:
			udp_print(cp, len, (packetbody_t)ip6, fragmented);
			return;
		case IPPROTO_ICMPV6:
			icmp6_print(ndo, cp, len, (packetbody_t)ip6, fragmented);
			return;
		case IPPROTO_AH:
			advance = ah_print(cp);
			nh = *cp;
			break;
		case IPPROTO_ESP:
		    {
			int enh, padlen;
			advance = esp_print(ndo, cp, len, (packetbody_t)ip6, &enh, &padlen);
			nh = enh & 0xff;
			len -= padlen;
			break;
		    }
		case IPPROTO_IPCOMP:
		    {
			int enh;
			advance = ipcomp_print(cp, &enh);
			nh = enh & 0xff;
			break;
		    }

		case IPPROTO_PIM:
			pim_print(cp, len, nextproto6_cksum(ip6, cp, len,
							    IPPROTO_PIM));
			return;

		case IPPROTO_OSPF:
			ospf6_print(cp, len);
			return;

		case IPPROTO_IPV6:
			ip6_print(ndo, cp, len);
			return;

		case IPPROTO_IPV4:
		        ip_print(ndo, cp, len);
			return;

                case IPPROTO_PGM:
                        pgm_print(cp, len, (packetbody_t)ip6);
                        return;

		case IPPROTO_GRE:
			gre_print(cp, len);
			return;

		case IPPROTO_RSVP:
			rsvp_print(cp, len);
			return;

		case IPPROTO_NONE:
			(void)ND_PRINT((ndo, "no next header"));
			return;

		default:
			(void)ND_PRINT((ndo, "ip-proto-%d %d", nh, len));
			return;
		}
	}

	return;
trunc:
	(void)ND_PRINT((ndo, "[|ip6]"));
}

#endif /* INET6 */
