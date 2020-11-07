# $Id: dirdeps-cache-update.mk,v 1.22 2020/09/10 00:14:38 sjg Exp $
#
#	@(#) Copyright (c) 2020, Simon J. Gerraty
#
#	This file is provided in the hope that it will
#	be of use.  There is absolutely NO WARRANTY.
#	Permission to copy, redistribute or otherwise
#	use this file is hereby granted provided that
#	the above copyright notice and this notice are
#	left intact.
#
#	Please send copies of changes and bug-fixes to:
#	sjg@crufty.net
#

##
#
# This makefile deals with the updating of STATIC_DIRDEPS_CACHE.
# Some targets are so huge that computing dirdeps takes a significant
# amount of time.  For such targets a STATIC_DIRDEPS_CACHE can make
# sense.
#
# If the target is represented by targets/pseudo/production
# it's normal DIRDEPS would be in
# targets/pseudo/production/Makefile.depend
# and STATIC_DIRDEPS_CACHE would be
# targets/pseudo/production/Makefile.dirdeps.cache
# which is simply initialized by copying dirdeps.cache.production
# from $OBJTOP
#
# When dirdeps-targets.mk is initializing DIRDEPS it will look for
# Makefile.dirdeps.cache and unless told not to
# (MK_STATIC_DIRDEPS_CACHE=no) will use it as DIRDEPS_CACHE.
#
# If MK_STATIC_DIRDEPS_CACHE_UPDATE is "yes", then this makefile
# comes into play.
#
# We usually get included from local.dirdeps.mk
# as well as Makefile.depend of RELDIR with a static Makefile.dirdeps.cache
#
# If we see that STATIC_DIRDEPS_CACHE is in use, we need to hook a
# cache-update target into the build to regenerate dirdeps.cache
# in parallel with the rest of the build.
# If MK_STATIC_DIRDEPS_CACHE_UPDATE_IMMEDIATE is "yes" we update
# STATIC_DIRDEPS_CACHE as soon as the update is ready,
# otherwise it will be done at the end of the build.
#
# If STATIC_DIRDEPS_CACHE is not in use, but a DIRDEPS_CACHE is,
# then we need do nothing except export STATIC_DIRDEPS_CACHE and
# DYNAMIC_DIRDEPS_CACHE for use when we are include during the visit
# to the ultimate target (targets/pseudo/production).
#
# Regardless of which happens, when included at .MAKE.LEVEL > 0
# for a target other than cache-update we simply copy
# DYNAMIC_DIRDEPS_CACHE to STATIC_DIRDEPS_CACHE with some optional
# filtering.
#
# If we are included for the target cache-update we take care of
# running dirdeps.mk again to generate the DYNAMIC_DIRDEPS_CACHE.
#

.if !target(_${.PARSEFILE}_)
_${.PARSEFILE}_: .NOTMAIN

STATIC_CACHE_SED += \
	-e '/Autogenerated/s,-.*,- edit with care!,' \
	-e '/cache-update/d'

STATIC_DIRDEPS_CACHE_UPDATE_SCRIPT ?= \
	{ echo Saving ${DYNAMIC_DIRDEPS_CACHE} as ${STATIC_DIRDEPS_CACHE}; \
        sed ${STATIC_CACHE_SED} ${DYNAMIC_DIRDEPS_CACHE} > ${STATIC_DIRDEPS_CACHE}; }
.endif

.if ${MK_DIRDEPS_CACHE:Uno} == "yes"
.if ${MK_STATIC_DIRDEPS_CACHE_UPDATE:Uno} == "yes"
.if ${_debug_reldir:U0} || ${DEBUG_DIRDEPS:U:Mcache*} != ""
_debug_cache = 1
.else
_debug_cache = 0
.endif

.if ${.MAKE.LEVEL} == 0 && !make(cache-update)

.if ${_debug_cache}
.info ${MK_STATIC_DIRDEPS_CACHE_UPDATE MK_STATIC_DIRDEPS_CACHE MK_DIRDEPS_CACHE DIRDEPS_CACHE STATIC_DIRDEPS_CACHE:L:@v@$v=${$v}@}
.endif

.if ${MK_STATIC_DIRDEPS_CACHE} == "yes" && defined(STATIC_DIRDEPS_CACHE) && exists(${STATIC_DIRDEPS_CACHE})
.if !make(dirdeps) && !target(cache_update_dirdep)
# We are using static cache and this is the only look we will get.
# We want to generate an updated cache while we build
# so need to hook cache-update to dirdeps now.
# Note: we are running as a sibling to dirdeps-cached,
# attempting to do this in that context is problematic.

# One of these should exist - to actually kick off the cache generation
.for d in ${STATIC_DIRDEPS_CACHE:H}/cache-update ${STATIC_DIRDEPS_CACHE:H:H}/cache-update ${STATIC_DIRDEPS_CACHE:H:H:H}/cache-update
.if exists($d)
cache_update_dirdep ?= $d.${TARGET_SPEC}
.endif
.endfor
dirdeps cache_update_dirdep: ${cache_update_dirdep}
${cache_update_dirdep}: _DIRDEP_USE
DYNAMIC_DIRDEPS_CACHE := ${OBJTOP}/dirdeps.cache.${STATIC_DIRDEPS_CACHE:H:T}-update
.export DYNAMIC_DIRDEPS_CACHE STATIC_DIRDEPS_CACHE
.endif	# make(dirdeps)
.endif	# MK_*

.endif	# .MAKE.LEVEL 0

.if ${.MAKE.LEVEL} > 0 && ${.CURDIR:T} == "cache-update"
# we are the background update shim

.if ${_debug_cache}
.info level ${.MAKE.LEVEL}: ${MK_DIRDEPS_CACHE DYNAMIC_DIRDEPS_CACHE STATIC_DIRDEPS_CACHE:L:@v@$v=${$v}@}
.endif

all: cache-build
cache-build: .META
	@set -x; MAKELEVEL=0 \
	${.MAKE} -C ${SRCTOP} -f ${RELDIR}/Makefile cache-update \
	-DWITHOUT_STATIC_DIRDEPS_CACHE_UPDATE

.endif	# cache-update

.elif ${.MAKE.LEVEL} == 0 && make(cache-update) && !target(cache-update)
# we were invoked above
# we just leverage dirdeps.mk
BUILD_DIRDEPS_TARGETS := ${STATIC_DIRDEPS_CACHE:H:T}
DIRDEPS := ${STATIC_DIRDEPS_CACHE:H:S,^${SRCTOP}/,,}.${TARGET_SPEC}
DIRDEPS_CACHE := ${DYNAMIC_DIRDEPS_CACHE}

.if ${DEBUG_DIRDEPS:U:Mcache*} != ""
.info level 0: ${MK_DIRDEPS_CACHE DIRDEPS_CACHE DIRDEPS:L:@v@$v=${$v}@}
.endif

# so cache-built below can check on us
x!= echo; echo ${.MAKE.PID} > ${DIRDEPS_CACHE}.new.pid

cache-update: ${DIRDEPS_CACHE}
	@rm -f ${DIRDEPS_CACHE}.new.pid
.if ${MK_STATIC_DIRDEPS_CACHE_UPDATE_IMMEDIATE:Uno} == "yes"
	${STATIC_DIRDEPS_CACHE_UPDATE_SCRIPT}
.endif

all:

.include <dirdeps.mk>

.endif	# MK_STATIC_DIRDEPS_CACHE_UPDATE
.endif	# MK_DIRDEPS_CACHE

.if ${.MAKE.LEVEL} > 0 && ${MK_STATIC_DIRDEPS_CACHE_UPDATE:Uno} == "yes" && \
	${STATIC_DIRDEPS_CACHE:Uno:H} == "${SRCTOP}/${RELDIR}"
.if !defined(DYNAMIC_DIRDEPS_CACHE)
all:
.else
# This is the easy bit, time to save the cache

all: cache-update

# ensure the cache update is completed
cache-built:
	@test -s ${DYNAMIC_DIRDEPS_CACHE}.new || exit 0; \
	pid=`cat ${DYNAMIC_DIRDEPS_CACHE}.new.pid 2> /dev/null`; \
	test $${pid:-0} -gt 1 || exit 0; \
	echo "Waiting for $$pid to finish ${DYNAMIC_DIRDEPS_CACHE} ..."; \
	while 'kill' -0 $$pid; do sleep 30; done > /dev/null 2>&1

cache-update: cache-built
.if ${MK_STATIC_DIRDEPS_CACHE_UPDATE_IMMEDIATE:Uno} == "no"
	@test ! -s ${DYNAMIC_DIRDEPS_CACHE} || \
	${STATIC_DIRDEPS_CACHE_UPDATE_SCRIPT}
.endif

.endif
.endif
