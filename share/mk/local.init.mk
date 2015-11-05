# $FreeBSD$

.if ${.MAKE.MODE:Unormal:Mmeta*} != ""
.if !empty(SUBDIR) && !defined(LIB) && !defined(PROG) && ${.MAKE.MAKEFILES:M*bsd.prog.mk} == ""
.if ${.MAKE.MODE:Mleaf*} != ""
# we only want leaf dirs to build in meta mode... and we are not one
.MAKE.MODE = normal
.endif
.endif
.endif

.if ${MK_SYSROOT} == "yes" && !empty(SYSROOT) && ${MACHINE} != "host"
CFLAGS_LAST+= --sysroot=${SYSROOT}
CXXFLAGS_LAST+= --sysroot=${SYSROOT}
LDADD+= --sysroot=${SYSROOT}
.elif ${MK_STAGING} == "yes"
CFLAGS+= -nostdinc
CFLAGS+= -I${STAGE_INCLUDEDIR}
LDADD+= -L${STAGE_LIBDIR}
.endif
.if ${MACHINE} == "host"
# we cheat?
LDADD+= -B/usr/lib
CFLAGS_LAST+= -I/usr/include
CXXFLAGS_LAST+= -I/usr/include
.endif

.if ${MACHINE} == "host"
.if ${.MAKE.DEPENDFILE:E} != "host"
UPDATE_DEPENDFILE?= no
.endif
HOST_CC?=	/usr/bin/cc
CC=		${HOST_CC}
HOST_CXX?=	/usr/bin/c++
CXX=		${HOST_CXX}
HOST_CPP?=	/usr/bin/cpp
CPP=		${HOST_CPP}
HOST_CFLAGS+= -DHOSTPROG
CFLAGS+= ${HOST_CFLAGS}
.endif

.if ${MK_CCACHE_BUILD} == "yes"
# Handle ccache after CC is determined.
# CC is always prepended with the ccache wrapper rather than modifying
# PATH since it is more clear that ccache is used.
LOCALBASE?=		/usr/local
CCACHE_WRAPPER_PATH?=	${LOCALBASE}/libexec/ccache
CCACHE_BIN?=		${LOCALBASE}/bin/ccache
.if exists(${CCACHE_BIN})
# Export to ensure sub-makes can filter it out for mkdep/linking and
# to chain down into kernel build which won't include this file.
.export CCACHE_BIN
# Handle bootstrapped compiler changes properly by hashing their content
# rather than checking mtime.  For external compilers it should be safe
# to use the more optimal mtime check.
# XXX: CCACHE_COMPILERCHECK= string:<compiler_version, compiler_build_rev, compiler_patch_rev, compiler_default_target, compiler_default_sysroot>
.if ${CC:N${CCACHE_BIN}:[1]:M/*} == ""
CCACHE_COMPILERCHECK?=	content
.else
CCACHE_COMPILERCHECK?=	mtime
.endif
.export CCACHE_COMPILERCHECK
# Remove ccache from the PATH to prevent double calls and wasted CPP/LD time.
PATH:=	${PATH:C,:?${CCACHE_WRAPPER_PATH}(/world)?(:$)?,,g}
.export PATH
# Override various toolchain vars.
.for var in CC CXX HOST_CC HOST_CXX
.if defined(${var}) && ${${var}:M${CCACHE_BIN}} == ""
${var}:=	${CCACHE_BIN} ${${var}}
.endif
.endfor
# GCC does not need the CCACHE_CPP2 hack enabled by default in devel/ccache.
# The port enables it due to ccache passing preprocessed C to clang
# which fails with -Wparentheses-equality, -Wtautological-compare, and
# -Wself-assign on macro-expanded lines.
.if defined(COMPILER_TYPE) && ${COMPILER_TYPE} == "gcc"
CCACHE_NOCPP2=	1
.export CCACHE_NOCPP2
.endif
# Canonicalize CCACHE_DIR for meta mode usage.
.if defined(CCACHE_DIR) && empty(.MAKE.META.IGNORE_PATHS:M${CCACHE_DIR})
CCACHE_DIR:=	${CCACHE_DIR:tA}
.MAKE.META.IGNORE_PATHS+= ${CCACHE_DIR}
.export CCACHE_DIR
.endif
.endif	# exists(${CCACHE_BIN})
.endif	# ${MK_CCACHE_BUILD} == "yes"
