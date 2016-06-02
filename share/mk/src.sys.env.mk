# $FreeBSD$

# early setup only see also src.sys.mk

# make sure this is defined in a consistent manner
SRCTOP:= ${.PARSEDIR:tA:H:H}

.if ${.CURDIR} == ${SRCTOP}
RELDIR = .
.elif ${.CURDIR:M${SRCTOP}/*}
RELDIR := ${.CURDIR:S,${SRCTOP}/,,}
.endif

# site customizations that do not depend on anything!
SRC_ENV_CONF?= /etc/src-env.conf
.if !empty(SRC_ENV_CONF) && !target(_src_env_conf_included_)
.-include "${SRC_ENV_CONF}"
_src_env_conf_included_:	.NOTMAIN
.endif

# Top-level installs should not use meta mode as it may prevent installing
# based on cookies.
.if make(*install*) && ${.MAKE.LEVEL} == 0
META_MODE=	normal
MK_META_MODE=	no
.endif

# Ignore some non-impactful host tools that will otherwise cause
# buildworld -> installworld -> buildworld to rebuild everything.
.MAKE.META.IGNORE_PATHS+= \
	${__MAKE_SHELL} \
	/bin \
	/rescue \
	/sbin \
	/usr/sbin \
	/usr/bin/awk \
	/usr/bin/basename \
	/usr/bin/cmp \
	/usr/bin/egrep \
	/usr/bin/env \
	/usr/bin/fgrep \
	/usr/bin/file2c \
	/usr/bin/gencat \
	/usr/bin/grep \
	/usr/bin/gzcat \
	/usr/bin/lex \
	/usr/bin/m4 \
	/usr/bin/mktemp \
	/usr/bin/nawk \
	/usr/bin/patch \
	/usr/bin/sed \
	/usr/bin/sort \
	/usr/bin/tail \
	/usr/bin/touch \
	/usr/bin/tr \
	/usr/bin/unifdef \
	/usr/bin/uudecode \

# Any dynamic binary used in the build will use host system libraries and
# rtld.  It is possible these libraries could impact the build output, but
# the actual binary using them will be detected and compared instead still.
.MAKE.META.IGNORE_PATHS+=	/lib /libexec /usr/lib
.MAKE.META.IGNORE_PATHS+=	/usr/share/locale /usr/share/zoneinfo
# For build-tools/cross-tools we can ignore the host compiler changing
# since these tools are self-reliant and only for bootstrapping the rest of the
# build.
.if defined(META_IGNORE_ALL_HOST)
.MAKE.META.IGNORE_PATHS+=	/usr/bin /usr/include
.endif

# If we were found via .../share/mk we need to replace that
# with ${.PARSEDIR:tA} so that we can be found by
# sub-makes launched from objdir.
.if ${.MAKEFLAGS:M.../share/mk} != ""
.MAKEFLAGS:= ${.MAKEFLAGS:S,.../share/mk,${.PARSEDIR:tA},}
.endif
.if ${MAKESYSPATH:Uno:M*.../*} != ""
MAKESYSPATH:= ${MAKESYSPATH:S,.../share/mk,${.PARSEDIR:tA},}
.export MAKESYSPATH
.elif empty(MAKESYSPATH)
MAKESYSPATH:=	${.PARSEDIR:tA}
.export MAKESYSPATH
.endif
