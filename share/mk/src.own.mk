# $FreeBSD$

# Note: This file is also duplicated in the sys/conf/kern.pre.mk so
# it will always grab SRCCONF, even if it isn't being built in-tree
# to preserve historical (and useful) behavior. Changes here need to
# be reflected there so SRCCONF isn't included multiple times.

.if !defined(_WITHOUT_SRCCONF)
# Allow user to configure things that only effect src tree builds.
SRCCONF?=	/etc/src.conf
.if (exists(${SRCCONF}) || ${SRCCONF} != "/etc/src.conf") && \
    !target(_srcconf_included_)

.sinclude "${SRCCONF}"
_srcconf_included_:	.NOTMAIN

# Validate the env-only variables.
.for var in ${__ENV_ONLY_OPTIONS}
__postrcconf_${var}:=	${MK_${var}:U-}${WITHOUT_${var}:Uno:Dyes}${WITH_${var}:Uno:Dyes}
.if ${__presrcconf_${var}} != ${__postrcconf_${var}}
.error Option ${var} may only be defined in ${SRC_ENV_CONF}, environment, or make argument, not ${SRCCONF}.
.endif
.undef __presrcconf_${var}
.undef __postrcconf_${var}
.endfor

.endif	# SRCCONF exists and not included.
.endif	# !defined(_WITHOUT_SRCCONF)
