# $FreeBSD$

# Search for kernel source tree in standard places.
.if empty(KERNBUILDDIR)
.if !defined(SYSDIR)
.for _dir in ${.CURDIR}/../.. ${.CURDIR}/../../.. ${.CURDIR}/../../../.. \
    ${.CURDIR}/../../../../.. /sys /usr/src/sys
.if exists(${_dir}/kern/) && exists(${_dir}/conf/kmod.mk)
SYSDIR=	${_dir:tA}
.endif
.endfor
.endif
.if !defined(SYSDIR) || !exists(${SYSDIR}/kern/) || \
    !exists(${SYSDIR}/conf/kmod.mk)
.error Unable to locate the kernel source tree. Set SYSDIR to override.
.endif
.endif

.include "${SYSDIR}/conf/kmod.mk"
