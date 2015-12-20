# $FreeBSD$
#
# You must include bsd.test.mk instead of this file from your Makefile.
#
# Internal glue for the build of /usr/tests/.

.if !target(__<bsd.test.mk>__)
.error suite.test.mk cannot be included directly.
.endif

# Name of the test suite these tests belong to.  Should rarely be changed for
# Makefiles built into the FreeBSD src tree.
TESTSUITE?= FreeBSD

# Knob to control the handling of the Kyuafile for this Makefile.
#
# If 'yes', a Kyuafile exists in the source tree and is installed into
# TESTSDIR.
#
# If 'auto', a Kyuafile is automatically generated based on the list of test
# programs built by the Makefile and is installed into TESTSDIR.  This is the
# default and is sufficient in the majority of the cases.
#
# If 'no', no Kyuafile is installed.
KYUAFILE?= auto

# Per-test program interface definition.
#
# The name provided here must match one of the interface names supported by
# Kyua as this is later encoded in the Kyuafile test program definitions.
#TEST_INTERFACE.<test-program>= interface-name

# Metadata properties applicable to all test programs.
#
# All the variables for a test program defined in the Makefile are appended
# to the test program's definition in the Kyuafile.  This feature can be
# used to avoid having to explicitly supply a Kyuafile in the source
# directory, allowing the caller Makefile to rely on the KYUAFILE=auto
# behavior defined here.
#TEST_METADATA+= key="value"

# Per-test program metadata properties as a list of key/value pairs.
#
# These per-test program settings _extend_ the values provided in the
# unqualified TEST_METADATA variable.
#TEST_METADATA.<test-program>+= key="value"

# Path to the prefix of the installed Kyua CLI, if any.
#
# If kyua is installed from ports, we automatically define a realtest target
# below to run the tests using this tool.  The tools are searched for in the
# hierarchy specified by this variable.
KYUA_PREFIX?= /usr/local

.if ${KYUAFILE:tl} != "no"
FILES+=	Kyuafile
FILESDIR_Kyuafile= ${TESTSDIR}
.endif

.if ${KYUAFILE:tl} == "auto"
CLEANFILES+= Kyuafile Kyuafile.auto.tmp
.endif

.for _T in ${_TESTS}
_TEST_METADATA.${_T}= ${TEST_METADATA} ${TEST_METADATA.${_T}}
.endfor

.if ${KYUAFILE:tl} == "auto"
Kyuafile: Makefile
	@{ \
	    echo '-- Automatically generated by bsd.test.mk.'; \
	    echo; \
	    echo 'syntax(2)'; \
	    echo; \
	    echo 'test_suite("${TESTSUITE}")'; \
            echo; \
	} > ${.TARGET}.tmp
.for _T in ${_TESTS}
	@echo '${TEST_INTERFACE.${_T}}_test_program{name="${_T}"${_TEST_METADATA.${_T}:C/$/,/:tW:C/^/, /W:C/,$//W}}' \
	    >>${.TARGET}.tmp
.endfor
.for _T in ${TESTS_SUBDIRS:N.WAIT}
	@echo "include(\"${_T}/${.TARGET}\")" >>${.TARGET}.tmp
.endfor
	@mv ${.TARGET}.tmp ${.TARGET}
.endif

KYUA?= ${KYUA_PREFIX}/bin/kyua
.if exists(${KYUA})
# Definition of the "make test" target and supporting variables.
#
# This target, by necessity, can only work for native builds (i.e. a FreeBSD
# host building a release for the same system).  The target runs Kyua, which is
# not in the toolchain, and the tests execute code built for the target host.
#
# Due to the dependencies of the binaries built by the source tree and how they
# are used by tests, it is highly possible for a execution of "make test" to
# report bogus results unless the new binaries are put in place.
realtest: .PHONY
	@echo "*** WARNING: make test is experimental"
	@echo "***"
	@echo "*** Using this test does not preclude you from running the tests"
	@echo "*** installed in ${TESTSBASE}.  This test run may raise false"
	@echo "*** positives and/or false negatives."
	@echo
	@${KYUA} test -k ${DESTDIR}${TESTSDIR}/Kyuafile; \
	result=0; \
	echo; \
	echo "*** Once again, note that "make test" is unsupported."; \
	test $${result} -eq 0
.endif

beforetest: .PHONY
.if defined(TESTSDIR)
.if ${TESTSDIR} == ${TESTSBASE}
# Forbid running from ${TESTSBASE}.  It can cause false positives/negatives and
# it does not cover all the tests (e.g. it misses testing software in external).
	@echo "*** Sorry, you cannot use make test from src/tests.  Install the"
	@echo "*** tests into their final location and run them from ${TESTSBASE}"
	@false
.else
	@echo "*** Using this test does not preclude you from running the tests"
	@echo "*** installed in ${TESTSBASE}.  This test run may raise false"
	@echo "*** positives and/or false negatives."
.endif
.else
	@echo "*** No TESTSDIR defined; nothing to do."
	@false
.endif
	@echo
