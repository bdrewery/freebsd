# $FreeBSD$

.include <src.opts.mk>

SUBDIR=		mbr pmbr boot0 boot0sio btx boot2 cdboot gptboot \
		libi386 libfirewire loader

.if ${COMPILER_TYPE} == "gcc" && ${COMPILER_VERSION} >= 40900
.warning Skipping build of boot2 as it fails with GCC.
SUBDIR:=	${SUBDIR:Nboot2}
.endif

# special boot programs, 'self-extracting boot2+loader'
SUBDIR+=	pxeldr

.if ${MACHINE_CPUARCH} == "i386"
SUBDIR+=	kgzldr
.endif

.if ${MK_ZFS} != "no"
SUBDIR+=	zfsboot gptzfsboot zfsloader
.endif

.include <bsd.subdir.mk>
