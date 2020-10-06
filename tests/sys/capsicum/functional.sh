#-
# SPDX-License-Identifier: BSD-2-Clause-FreeBSD
#
# Copyright (c) 2020 The FreeBSD Foundation
#
# This software was developed by Li-Wen Hsu <lwhsu@FreeBSD.org>
# under sponsorship from the FreeBSD Foundation.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $FreeBSD$

SRCDIR=$(atf_get_srcdir)
CAPSICUM_TEST_BIN=capsicum-test

check()
{
	local tc=${1}

	atf_check -s exit:0 -o match:PASSED  -e ignore \
		${SRCDIR}/${CAPSICUM_TEST_BIN} --gtest_filter=${tc}
}

add_testcase()
{
	local tc=${1}
	local tc_escaped word

	tc_escaped=$(echo ${tc} | sed -e 's/\./__/')

	atf_test_case ${tc_escaped}
	eval "${tc_escaped}_body() { check ${tc}; }"
	atf_add_test_case ${tc_escaped}
}

list_tests()
{
	${SRCDIR}/${CAPSICUM_TEST_BIN} --gtest_list_tests | awk '
		/^[^ ]/ { CAT=$0 }
		/^[ ]/ { print CAT $1}'
}

atf_init_test_cases()
{
	local t
	for t in `list_tests`; do
		add_testcase $t
	done
}
