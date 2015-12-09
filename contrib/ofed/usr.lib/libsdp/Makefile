# $FreeBSD$

SHLIBDIR?=	/usr/lib

.include <bsd.own.mk>

SDPDIR=	${.CURDIR}/../../libsdp/src

.PATH: ${SDPDIR}

LIB=		ibsdp
SHLIB_MAJOR=	1
MK_PROFILE=	no
# Has optional symbols program_invocation_short_name and __sdp_sockaddr_to_sdp
MK_LINKTEST=	no
MAN=

SRCS=	log.c match.c port.c config_parser.c config_scanner.c

CFLAGS+=	-DSYSCONFDIR=\"/etc\"
CFLAGS+= -I${OFEDSYS}/include

.include <bsd.lib.mk>
