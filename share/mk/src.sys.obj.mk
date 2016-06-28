# $FreeBSD$
#
# Early setup of MAKEOBJDIR
#
# Default is like: /usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]bin/sh
#  MAKEOBJDIRPREFIX is	/usr/obj
#  OBJROOT is		/usr/obj/usr/src/
#  OBJTOP is		/usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]
#    XXX: The TARGET.TARGET_ARCH is only used for cross-builds currently.
#  MAKEOBJDIR is	/usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]bin/sh
#

_default_MAKEOBJDIRPREFIX?=	/usr/obj
_default_MAKEOBJDIR=	$${.CURDIR:S,^$${SRCTOP},$${OBJTOP},}

.if empty(OBJROOT) || ${.MAKE.LEVEL} == 0
.if !empty(MAKEOBJDIRPREFIX)
OBJROOT:=	${MAKEOBJDIRPREFIX}${SRCTOP}/
MAKEOBJDIRPREFIX=
.export MAKEOBJDIRPREFIX
.endif
.if empty(MAKEOBJDIR)
# OBJTOP set below
MAKEOBJDIR=	${_default_MAKEOBJDIR}
# export but do not track
.export-env MAKEOBJDIR
# Expand for our own use
MAKEOBJDIR:=	${MAKEOBJDIR}
.endif
# XXX: Undocumented SandBox feature
.if !empty(SB)
SB_OBJROOT?=	${SB}/obj/
# this is what we use below
OBJROOT?=	${SB_OBJROOT}
.endif
OBJROOT?=	${_default_MAKEOBJDIRPREFIX}${SRCTOP}/
.if ${OBJROOT:M*/} != ""
OBJROOT:=	${OBJROOT:H:tA}/
.else
OBJROOT:=	${OBJROOT:H:tA}/${OBJROOT:T}
.endif
# Must export since OBJDIR will dynamically be based on it
.export OBJROOT SRCTOP
.endif

# XXX: Only use TARGET.TARGET_ARCH for cross-builds currently.
.if defined(TARGET) && defined(TARGET_ARCH) && \
    !(${MACHINE} == ${TARGET} && ${MACHINE_ARCH} == ${TARGET_ARCH} && \
    !defined(CROSS_BUILD_TESTING))
OBJTOP:=	${OBJROOT}${TARGET}.${TARGET_ARCH}
.else
OBJTOP:=	${OBJROOT:H}
.endif
