# $FreeBSD$

CFLAGS+=	-I${WORLDTMP}/legacy/usr/include
DPADD+=		${WORLDTMP}/legacy/usr/lib/libegacy.a
LDADD+=		-legacy
LDFLAGS+=	-L${WORLDTMP}/legacy/usr/lib

.if ${.MAKE.OS} != "FreeBSD"
# On MacOS using a non-mac ar will fail the build, similarly on Linux using
# nm may not work as expected if the nm for the target architecture comes in
# $PATH before a nm that supports the host architecture.
# To ensure that host binary compile as expected we use the tools from /usr/bin.
AR:=	/usr/bin/ar
RANLIB:=	/usr/bin/ranlib
NM:=	/usr/bin/nm

# Don't use lorder and tsort since lorder is not installed by default on most
# Linux systems and the FreeBSD lorder does not work on Linux. For the bootstrap
# tools the order of the .o files should not matter since we only care about
# a few individual files (and might soon require linking with lld anyway)
LORDER:=echo
TSORT:=cat
# When using cat as tsort we can't pass -q:
TSORTFLAGS:=

# Avoid stale dependecy warnings:
LIBC:=
LIBZ:=
LIBM:=
LIBUTIL:=
LIBCPLUSPLUS:=
LIBARCHIVE:=
LIBPTHREAD:=
LIBMD:=${WORLDTMP}/legacy/usr/lib/libmd.a
LIBNV:=${WORLDTMP}/legacy/usr/lib/libmd.a
LIBSBUF:=${WORLDTMP}/legacy/usr/lib/libsbuf.a
LIBY:=${WORLDTMP}/legacy/usr/lib/liby.a
LIBL:=${WORLDTMP}/legacy/usr/lib/libl.a
LIBROKEN:=${WORLDTMP}/legacy/usr/lib/libroken.a
LIBDWARF:=${WORLDTMP}/legacy/usr/lib/libdwarf.a
LIBELF:=${WORLDTMP}/legacy/usr/lib/libelf.a

# Add various -Werror flags to catch missing function declarations
CFLAGS+=	-Werror=implicit-function-declaration -Werror=implicit-int \
		-Werror=return-type -Wundef
CFLAGS+=	-DHAVE_NBTOOL_CONFIG_H=1
CFLAGS+=	-D__BSD_VISIBLE=1
CFLAGS+=	-I${SRCTOP}/tools/build/cross-build/include/common

# b64_pton and b64_ntop is in libresolv on MacOS and Linux:
# TODO: only needed for uuencode and uudecode
LDADD+=-lresolv

.if ${.MAKE.OS} == "Linux"
CFLAGS+=	-I${SRCTOP}/tools/build/cross-build/include/linux
CFLAGS+=	-std=gnu99 -D_GNU_SOURCE=1
# Needed for sem_init, etc. on Linux (used by usr.bin/sort)
LDADD+=	-pthread

.elif ${.MAKE.OS} == "Darwin"
CFLAGS+=	-D_DARWIN_C_SOURCE=1
CFLAGS+=	-I${SRCTOP}/tools/build/cross-build/include/mac
# The macOS ar and ranlib don't understand all the flags supported by the
# FreeBSD and Linux ar/ranlib
ARFLAGS:=	-crs
RANLIBFLAGS:=

# to get libarchive (needed for elftoolchain)
# MacOS ships /usr/lib/libarchive.dylib but doesn't provide the headers
CFLAGS+=	-idirafter ${SRCTOP}/contrib/libarchive/libarchive
.else
.error "Unsupported build OS: ${.MAKE.OS}"
.endif
.endif # ${.MAKE.OS} != "FreeBSD"

# we do not want to capture dependencies referring to the above
UPDATE_DEPENDFILE= no

# When building host tools we should never pull in headers from the source sys
# directory to avoid any ABI issues that might cause the built binary to crash.
# The only exceptions to this are sys/cddl/compat for dtrace bootstrap tools and
# sys/crypto for libmd bootstrap.
# We have to skip this check during make obj since bsd.crunchgen.mk will run
# make obj on every directory during the build-tools phase.
.if !make(obj)
.if !empty(CFLAGS:M*${SRCTOP}/sys)
.error Do not include $${SRCTOP}/sys when building bootstrap tools. \
    Copy the header to $${WORLDTMP}/legacy in tools/build/Makefile instead. \
    Error was caused by Makefile in ${.CURDIR}
.endif

# ${SRCTOP}/include should also never be used to avoid ABI issues
.if !empty(CFLAGS:M*${SRCTOP}/include*)
.error Do not include $${SRCTOP}/include when building bootstrap tools. \
    Copy the header to $${WORLDTMP}/legacy in tools/build/Makefile instead. \
    Error was caused by Makefile in ${.CURDIR}
.endif
.endif
