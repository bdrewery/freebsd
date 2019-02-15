#!/usr/local/bin/ksh93 -p
#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

# $FreeBSD$

#
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# ident	"@(#)zpool_history_001_neg.ksh	1.2	07/01/09 SMI"
#

. $STF_SUITE/include/libtest.kshlib

#################################################################################
#
# __stc_assertion_start
#
# ID: zpool_history_001_neg
#
# DESCRIPTION:
#	Verify 'zpool history' can deal with non-existent pools and garbage 
#	to the command. 
#
# STRATEGY:
#	1. Create pool, volume & snap
#	2. Verify 'zpool history' can cope with incorret arguments.
#
# TESTABILITY: explicit
#
# TEST_AUTOMATION_LEVEL: automated
#
# CODING_STATUS: COMPLETED (2006-07-07)
#
# __stc_assertion_end
#
################################################################################

verify_runnable "global"

snap=$TESTPOOL/$TESTFS@snap
clone=$TESTPOOL/clone

set -A neg_opt "$TESTPOOL/$TESTCTR" "$TESTPOOL/$TESTVOL" "-t $TESTPOOL" \
	"-v $TESTPOOL" "$snap" "$clone" "nonexist" "TESTPOOL"

function cleanup
{
	datasetexists $clone && log_must $ZFS destroy $clone
	datasetexists $snap && log_must $ZFS destroy $snap
}

log_assert "Verify 'zpool history' can deal with non-existent pools and " \
	"garbage to the command."
log_onexit cleanup

log_must $ZFS snapshot $snap
log_must $ZFS clone $snap $clone

for opt in "${neg_opt[@]}"; do
	log_mustnot eval "$ZPOOL history $opt > /dev/null"
done

log_pass "'zpool history' command line negation test passed."
