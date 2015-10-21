# $FreeBSD$

.PATH: ${.CURDIR}/../../sys/geom

PROG=	sunlabel
SRCS=	sunlabel.c geom_sunlabel_enc.c
MAN=	sunlabel.8

.if ${MACHINE_CPUARCH} == "sparc64"
LINKS=	${BINDIR}/sunlabel ${BINDIR}/disklabel
MLINKS=	sunlabel.8 disklabel.8
.endif

LIBADD=	geom

.include <bsd.prog.mk>

test:	${PROG}
	${SH} ${.CURDIR}/runtest.sh

testx: ${PROG}
	${SH} -x ${.CURDIR}/runtest.sh
