# $FreeBSD$

TESTSDIR=	${TESTSBASE}/sys/kern

ATF_TESTS_C+=	kern_copyin
ATF_TESTS_C+=	kern_descrip_test
ATF_TESTS_C+=	ptrace_test
ATF_TESTS_C+=	unix_seqpacket_test
TEST_METADATA.unix_seqpacket_test+=	timeout="15"

LIBADD.ptrace_test+=			pthread
LIBADD.unix_seqpacket_test+=		pthread

WARNS?=	5

TESTS_SUBDIRS+=	execve

.include <bsd.test.mk>
