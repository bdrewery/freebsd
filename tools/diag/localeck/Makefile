# $FreeBSD$

PROG=	localeck
MAN=
WARNS?=	4

LOCALEDIR?= /usr/share/locale

test: ${PROG}
	${SH} docheck.sh ${LOCALEDIR}

.include <bsd.prog.mk>
