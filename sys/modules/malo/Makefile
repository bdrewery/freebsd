# $FreeBSD$

.PATH: ${SRCTOP}/sys/dev/malo

KMOD    = if_malo
SRCS    = if_malo.c if_malohal.c if_malo_pci.c
SRCS+=	device_if.h bus_if.h pci_if.h opt_malo.h

.if !defined(KERNBUILDDIR)
opt_malo.h:
	echo '#define MALO_DEBUG 1'> $@
.endif

.include <bsd.kmod.mk>
