# $FreeBSD$

.include <bsd.own.mk>

ATF_TESTS_C=		arc4random_test
ATF_TESTS_C+=		fpclassify2_test

# TODO: t_closefrom, t_cpuset, t_fmtcheck, t_randomid, t_sleep
# TODO: t_siginfo (fixes require further inspection)
# TODO: t_sethostname_test (consistently screws up the hostname)

NETBSD_ATF_TESTS_C=	alarm_test
NETBSD_ATF_TESTS_C+=	assert_test
NETBSD_ATF_TESTS_C+=	basedirname_test
NETBSD_ATF_TESTS_C+=	dir_test
NETBSD_ATF_TESTS_C+=	floatunditf_test
NETBSD_ATF_TESTS_C+=	fnmatch_test
NETBSD_ATF_TESTS_C+=	fpclassify_test
NETBSD_ATF_TESTS_C+=	fpsetmask_test
NETBSD_ATF_TESTS_C+=	fpsetround_test
NETBSD_ATF_TESTS_C+=	ftok_test
NETBSD_ATF_TESTS_C+=	getcwd_test
NETBSD_ATF_TESTS_C+=	getgrent_test
NETBSD_ATF_TESTS_C+=	glob_test
NETBSD_ATF_TESTS_C+=	humanize_number_test
NETBSD_ATF_TESTS_C+=	isnan_test
NETBSD_ATF_TESTS_C+=	nice_test
NETBSD_ATF_TESTS_C+=	pause_test
NETBSD_ATF_TESTS_C+=	raise_test
NETBSD_ATF_TESTS_C+=	realpath_test
NETBSD_ATF_TESTS_C+=	setdomainname_test
NETBSD_ATF_TESTS_C+=	sethostname_test
NETBSD_ATF_TESTS_C+=	sleep_test
NETBSD_ATF_TESTS_C+=	syslog_test
NETBSD_ATF_TESTS_C+=	time_test
NETBSD_ATF_TESTS_C+=	ttyname_test
NETBSD_ATF_TESTS_C+=	vis_test

.include "../Makefile.netbsd-tests"

LIBADD.humanize_number_test+=	util

LIBADD.fpclassify_test+=m
LIBADD.fpsetround_test+=m
LIBADD.siginfo_test+=	m

LIBADD.nice_test+=	pthread
LIBADD.syslog_test+=	pthread

TESTS_SUBDIRS=	execve
TESTS_SUBDIRS+=	posix_spawn

.include <bsd.test.mk>
