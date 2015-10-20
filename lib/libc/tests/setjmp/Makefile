# $FreeBSD$

NETBSD_ATF_TESTS_C=	t_setjmp
NETBSD_ATF_TESTS_C+=	t_threadjmp

LIBADD.t_threadjmp+=	pthread

WARNS?=	4

.include "../Makefile.netbsd-tests"

.include <bsd.test.mk>
