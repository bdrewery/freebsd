# $FreeBSD$
#
# Early setup of MAKEOBJDIR
#
# Default format is: /usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]bin/sh
#  MAKEOBJDIRPREFIX is	/usr/obj
#  OBJROOT is		/usr/obj/usr/src/
#  OBJTOP is		/usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]
#  MAKEOBJDIR is	/usr/obj/usr/src/[${TARGET}.${TARGET_ARCH}/]bin/sh
#
#  If MK_UNIFIED_OBJDIR is no then OBJROOT will always match OBJTOP.
#
#  If .MAKE.LEVEL == 0 then the TARGET.TARGET_ARCH is potentially added on.
#  If .MAKE.LEVEL >  0 and MAKEOBJDIRPREFIX is set then it will not get
#  TARGET.TARGET_ARCH added in as it assumes that MAKEOBJDIRPREFIX is
#  nested in the existing OBJTOP with TARGET.TARGET_ARCH in it.
#

_default_makeobjdirprefix?=	/usr/obj
_default_makeobjdir=	$${.CURDIR:S,^$${SRCTOP},$${OBJTOP},}

.if ${MK_UNIFIED_OBJDIR} == "no"
# Fall back to simpler/older behavior
MAKEOBJDIRPREFIX?=	${_default_makeobjdirprefix}
.if !(defined(TARGET) && defined(TARGET_ARCH) && \
    ${MACHINE} == ${TARGET} && ${MACHINE_ARCH} == ${TARGET_ARCH} && \
    !defined(CROSS_BUILD_TESTING))
MAKEOBJDIRPREFIX:=	${MAKEOBJDIRPREFIX}${TARGET:D/${TARGET}.${TARGET_ARCH}}
.endif
.endif	# ${MK_UNIFIED_OBJDIR} == "no"

# Allow passing in OBJTOP only
.if !empty(OBJTOP) && !defined(OBJROOT)
OBJROOT:=	${OBJTOP}/
.endif

.if ${.MAKE.LEVEL} == 0 || (empty(MAKEOBJDIRPREFIX) && empty(OBJROOT))
.if !empty(MAKEOBJDIRPREFIX)
# put things approximately where they want
OBJROOT:=	${MAKEOBJDIRPREFIX}${SRCTOP}/
MAKEOBJDIRPREFIX=
.export MAKEOBJDIRPREFIX
.endif
.if empty(MAKEOBJDIR)
# OBJTOP set below
MAKEOBJDIR=	${_default_makeobjdir}
# export but do not track
.export-env MAKEOBJDIR
# Expand for our own use
MAKEOBJDIR:=	${MAKEOBJDIR}
.endif
# XXX: Undocumented SB feature
.if !empty(SB)
SB_OBJROOT?=	${SB}/obj/
# this is what we use below
OBJROOT?=	${SB_OBJROOT}
.endif
OBJROOT?=	${_default_makeobjdirprefix}${SRCTOP}/
.if ${OBJROOT:M*/} != ""
OBJROOT:=	${OBJROOT:H:tA}/
.else
OBJROOT:=	${OBJROOT:H:tA}/${OBJROOT:T}
.endif
# Must export since OBJDIR will dynamically be based on it
.export OBJROOT SRCTOP
.elif !empty(OBJROOT) && !empty(MAKEOBJDIRPREFIX)
.error Both OBJROOT=${OBJROOT} and MAKEOBJDIRPREFIX=${MAKEOBJDIRPREFIX} are set
.endif

# OBJTOP normally won't be set yet unless passed in environment or using
# the old style objdir.
.if empty(OBJTOP)
.if ${MK_UNIFIED_OBJDIR} == "yes"
OBJTOP:=	${OBJROOT}${TARGET:D${TARGET}.${TARGET_ARCH}:U${MACHINE}.${MACHINE_ARCH}}
.else
# TARGET.TARGET_ARCH handled in OBJROOT already.
OBJTOP:=	${OBJROOT:H}
.endif	# ${MK_UNIFIED_OBJDIR} == "yes"
.endif	# empty(OBJTOP)

# Wait to validate MAKEOBJDIR until OBJTOP is set.
.if defined(MAKEOBJDIR)
.if ${MAKEOBJDIR:M/*} == ""
.error Cannot use MAKEOBJDIR=${MAKEOBJDIR}${.newline}Unset MAKEOBJDIR to get default:  MAKEOBJDIR='${_default_makeobjdir}'
.endif
.endif

# Assign this directory as .OBJDIR if possible
#
# The expected OBJDIR already exists, set it as .OBJDIR.
.if !empty(MAKEOBJDIRPREFIX) && exists(${MAKEOBJDIRPREFIX}${.CURDIR})
.OBJDIR: ${MAKEOBJDIRPREFIX}${.CURDIR}
.elif exists(${MAKEOBJDIR})
.OBJDIR: ${MAKEOBJDIR}
# Special case to work around bmake bug.  If the top-level .OBJDIR does not yet
# exist and MAKEOBJDIR is passed into environment and yield a blank value,
# bmake will incorrectly set .OBJDIR=${SRCTOP}/ rather than the expected
# ${SRCTOP} to match ${.CURDIR}.
.elif ${MK_AUTO_OBJ} == "no" && ${.CURDIR} == ${SRCTOP}
.OBJDIR: ${.CURDIR}
.endif
