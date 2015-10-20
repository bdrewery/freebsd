# $FreeBSD$

.include <bsd.own.mk>

# TODO: popcount, stresep

NETBSD_ATF_TESTS_C+=	memchr
NETBSD_ATF_TESTS_C+=	memcpy
NETBSD_ATF_TESTS_C+=	memmem
NETBSD_ATF_TESTS_C+=	memset
NETBSD_ATF_TESTS_C+=	strcat
NETBSD_ATF_TESTS_C+=	strchr
NETBSD_ATF_TESTS_C+=	strcmp
NETBSD_ATF_TESTS_C+=	strcpy
NETBSD_ATF_TESTS_C+=	strcspn
NETBSD_ATF_TESTS_C+=	strerror
NETBSD_ATF_TESTS_C+=	strlen
NETBSD_ATF_TESTS_C+=	strpbrk
NETBSD_ATF_TESTS_C+=	strrchr
NETBSD_ATF_TESTS_C+=	strspn
NETBSD_ATF_TESTS_C+=	swab

.include "../Makefile.netbsd-tests"

LIBADD.memchr+=	md

LIBADD.memcpy+=	md

.include <bsd.test.mk>
