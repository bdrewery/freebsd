# $FreeBSD$

SHLIBDIR?=	/usr/lib

.include <bsd.own.mk>
.include "../Makefile.inc"

.PATH: ${OPENSMDIR}/opensm

LIB=		opensm
SHLIB_MAJOR=	1
MK_PROFILE=	no

SRCS=	osm_log.c osm_mad_pool.c osm_helper.c

LIBADD= pthread

VERSION_MAP= ${OPENSMDIR}/opensm/libopensm.map

# Has cyclic dependency with osmvendor and osmcomp
MK_LINKTEST=	no

.include <bsd.lib.mk>
