/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2006 Alexander Motin <mav@alkar.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
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
 */

#ifndef _NETGRAPH_NG_PRED1_H_
#define _NETGRAPH_NG_PRED1_H_

/* Node type name and magic cookie */
#define NG_PRED1_NODE_TYPE	"pred1"
#define NGM_PRED1_COOKIE	1166902612

/* Hook names */
#define NG_PRED1_HOOK_COMP	"comp"		/* compression hook */
#define NG_PRED1_HOOK_DECOMP	"decomp"	/* decompression hook */

/* Config struct */
struct ng_pred1_config {
	u_char		enable;			/* node enabled */
};

/* Keep this in sync with the above structure definition. */
#define NG_PRED1_CONFIG_INFO	{			\
	{ "enable",	&ng_parse_uint8_type	},	\
	{ NULL }					\
}

/* Statistics structure for one direction. */
struct ng_pred1_stats {
	uint64_t	FramesPlain;
	uint64_t	FramesComp;
	uint64_t	FramesUncomp;
	uint64_t	InOctets;
	uint64_t	OutOctets;
	uint64_t	Errors;
};

/* Keep this in sync with the above structure definition. */
#define NG_PRED1_STATS_INFO	{				\
	{ "FramesPlain",&ng_parse_uint64_type	},	\
	{ "FramesComp",	&ng_parse_uint64_type	},	\
	{ "FramesUncomp", &ng_parse_uint64_type	},	\
	{ "InOctets",	&ng_parse_uint64_type	},	\
	{ "OutOctets",	&ng_parse_uint64_type	},	\
	{ "Errors",	&ng_parse_uint64_type	},	\
	{ NULL }					\
}

/* Netgraph commands */
enum {
	NGM_PRED1_CONFIG = 1,
	NGM_PRED1_RESETREQ,			/* sent either way! */
	NGM_PRED1_GET_STATS,
	NGM_PRED1_CLR_STATS,
	NGM_PRED1_GETCLR_STATS,
};

#endif /* _NETGRAPH_NG_PRED1_H_ */
