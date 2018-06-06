# $FreeBSD$

_spath=${SRCTOP}/contrib/ofed/librdmacm
.PATH: ${_spath}/examples ${_spath}/man

BINDIR?=	/usr/bin
CFLAGS+=	-I${SRCTOP}/contrib/ofed
LDFLAGS+=	-libverbs -lrdmacm -lmlx4 -lmlx5 -lcxgb4 -lpthread
