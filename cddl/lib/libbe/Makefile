# $FreeBSD$

.PATH: ${.CURDIR}/../../contrib/opensolaris/lib/libbe/common

LIB=	be
SRCS=	be_activate.c be_create.c be_list.c be_mount.c be_rename.c \
	be_snapshot.c be_utils.c be_zones.c
#INCS=	libbe.h libbe_priv.h
WARNS?=	1
CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${OPENSOLARIS_USR_DISTDIR}/lib/libbe/common
CFLAGS+= -I${.CURDIR}/../../compat/opensolaris/include
CFLAGS+= -I${.CURDIR}/../../../sys/cddl/compat/opensolaris
CFLAGS+= -I${OPENSOLARIS_USR_DISTDIR}/cmd/boot/common
CFLAGS+= -I${OPENSOLARIS_USR_DISTDIR}/lib/libzfs/common
CFLAGS+= -I${OPENSOLARIS_USR_DISTDIR}/lib/libzfs_core/common
CFLAGS+= -I${OPENSOLARIS_USR_DISTDIR}/lib/libnvpair
#CFLAGS+= -I${OPENSOLARIS_SYS_DISTDIR}/common/zfs
CFLAGS+= -I${OPENSOLARIS_SYS_DISTDIR}/uts/common
CFLAGS+= -I${OPENSOLARIS_SYS_DISTDIR}/uts/common/fs/zfs
#CFLAGS+= -I${OPENSOLARIS_SYS_DISTDIR}/uts/common/fs/zfs/sys

LIBADD+=	zfs
#LIBADD+=	instzones
#LIBADD+=	uuid
LIBADD+=	nvpair
LIBADD+=	devinfo
#LIBADD+=	efi

CSTD=	c99

.include <bsd.lib.mk>
