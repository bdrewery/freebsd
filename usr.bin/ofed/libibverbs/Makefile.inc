# $FreeBSD$

_spath=${SRCTOP}/contrib/ofed/libibverbs
.PATH: ${_spath}/examples ${_spath}/man

BINDIR?=	/usr/bin
CFLAGS+=	-I${_spath}
LDFLAGS+=	-libverbs -lmlx4 -lmlx5 -lcxgb4 -lpthread

