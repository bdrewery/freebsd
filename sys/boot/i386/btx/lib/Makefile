# $FreeBSD$

PROG=	crt0.o
INTERNALPROG=
MAN=
SRCS=	btxcsu.S btxsys.s btxv86.s
CFLAGS+=-I${.CURDIR}/../../common
LDFLAGS=-Wl,-r

#CFLAGS+=	-fpic
NO_PIE=	yes	# avoid -fpie due to using bsd.prog.mk

.include <bsd.prog.mk>
